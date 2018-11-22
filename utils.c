#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "port.h"
#include "utils.h"

void str_error(int errnum, const char *file, int line, const char *func)
{
  int size = 256;
  char *err;
  char buf[256 * 2];

  err = strerror(errnum);
  snprintf(buf, size * 2,
           "[ERROR] %s:%i at %s(): %s", file, line, func, err);

  printf("%s\n", buf);
  fflush(stdout);
}

OList *o_list_new(void *data){
	OList *new_elem=(OList*)os_new0(OList,1);
	new_elem->data=data;
	return new_elem;
}

OList * o_list_append(OList *elem, void * data){
	OList *new_elem=o_list_new(data);
	OList *it=elem;
	if (elem==NULL) return new_elem;
	while (it->next!=NULL) it=o_list_next(it);
	it->next=new_elem;
	new_elem->prev=it;
	return elem;
}

OList * o_list_free(OList *list){
	OList *elem = list;
	OList *tmp;
	return_val_if_fail(list, list);
	while(elem->next!=NULL) {
		tmp = elem;
		elem = elem->next;
		os_free(tmp);
	}
	os_free(elem);
	return NULL;
}

OList *o_list_remove_link(OList *list, OList *elem){
	OList *ret;
	if (elem==list){
		ret=elem->next;
		elem->prev=NULL;
		elem->next=NULL;
		if (ret!=NULL) ret->prev=NULL;
		os_free(elem);
		return ret;
	}
	elem->prev->next=elem->next;
	if (elem->next!=NULL) elem->next->prev=elem->prev;
	elem->next=NULL;
	elem->prev=NULL;
	os_free(elem);
	return list;
}

OList * o_list_remove(OList *list, void *data){
	OList *it;
	for(it=list;it!=NULL;it=it->next){
		if (it->data==data){
			return o_list_remove_link(list,it);
		}
	}
	return list;
}
