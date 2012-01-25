/*
	belle-sip - SIP (RFC3261) library.
    Copyright (C) 2010  Belledonne Communications SARL

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "belle_sip_internal.h"


static void belle_sip_stack_destroy(belle_sip_stack_t *stack){
	belle_sip_object_unref(stack->ml);
	belle_sip_list_for_each (stack->lp,belle_sip_object_unref);
	belle_sip_list_free(stack->lp);
}

BELLE_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(belle_sip_stack_t);
BELLE_SIP_INSTANCIATE_VPTR(belle_sip_stack_t,belle_sip_object_t,belle_sip_stack_destroy,NULL,NULL,FALSE);

belle_sip_stack_t * belle_sip_stack_new(const char *properties){
	belle_sip_stack_t *stack=belle_sip_object_new(belle_sip_stack_t);
	stack->ml=belle_sip_main_loop_new ();
	stack->timer_config.T1=500;
	stack->timer_config.T2=4000;
	stack->timer_config.T4=5000;
	return stack;
}

const belle_sip_timer_config_t *belle_sip_stack_get_timer_config(const belle_sip_stack_t *stack){
	return &stack->timer_config;
}

belle_sip_listening_point_t *belle_sip_stack_create_listening_point(belle_sip_stack_t *s, const char *ipaddress, int port, const char *transport){
	belle_sip_listening_point_t *lp=NULL;
	if (strcasecmp(transport,"UDP")==0){
		lp=belle_sip_udp_listening_point_new(s,ipaddress,port);
	}else{
		belle_sip_fatal("Unsupported transport %s",transport);
	}
	if (lp!=NULL){
		s->lp=belle_sip_list_append(s->lp,lp);
	}
	return lp;
}

void belle_sip_stack_delete_listening_point(belle_sip_stack_t *s, belle_sip_listening_point_t *lp){
	s->lp=belle_sip_list_remove(s->lp,lp);
	belle_sip_object_unref(lp);
}

belle_sip_provider_t *belle_sip_stack_create_provider(belle_sip_stack_t *s, belle_sip_listening_point_t *lp){
	belle_sip_provider_t *p=belle_sip_provider_new(s,lp);
	return p;
}

void belle_sip_stack_delete_provider(belle_sip_stack_t *s, belle_sip_provider_t *p){
	belle_sip_object_unref(p);
}

belle_sip_main_loop_t * belle_sip_stack_get_main_loop(belle_sip_stack_t *stack){
	return stack->ml;
}

void belle_sip_stack_main(belle_sip_stack_t *stack){
	belle_sip_main_loop_run(stack->ml);
}

void belle_sip_stack_sleep(belle_sip_stack_t *stack, unsigned int milliseconds){
	belle_sip_main_loop_sleep (stack->ml,milliseconds);
}

void belle_sip_stack_get_next_hop(belle_sip_stack_t *stack, belle_sip_request_t *req, belle_sip_hop_t *hop){
	belle_sip_header_route_t *route=BELLE_SIP_HEADER_ROUTE(belle_sip_message_get_header(BELLE_SIP_MESSAGE(req),"route"));
	belle_sip_uri_t *uri;
	if (route!=NULL){
		uri=belle_sip_header_address_get_uri(BELLE_SIP_HEADER_ADDRESS(route));
	}else{
		uri=belle_sip_request_get_uri(req);
	}
	hop->transport=belle_sip_uri_get_transport_param(uri);
	if (hop->transport==NULL) hop->transport="UDP";
	hop->host=belle_sip_uri_get_host(uri);
	hop->port=belle_sip_uri_get_listening_port(uri);
}

unsigned int belle_sip_random(void){
#ifdef __linux
	static int fd=-1;
	if (fd==-1) fd=open("/dev/urandom",O_RDONLY);
	if (fd!=-1){
		unsigned int tmp;
		if (read(fd,&tmp,4)!=4){
			belle_sip_error("Reading /dev/urandom failed.");
		}else return tmp;
	}else belle_sip_error("Could not open /dev/urandom");
#endif
	return (unsigned int) random();
}
