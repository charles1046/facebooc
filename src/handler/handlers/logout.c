#include "handler/handlers/logout.h"

#include "handler/handler_utility.h"
#include "models/account.h"
#include "utility.h"

Response* logout(Request* req) {
	const Account* my_acc = get_account(req->cookies);
	if(unlikely(my_acc == NULL)) {	// It's usually logged in
		accountDel((Account*)my_acc);
		return responseNewRedirect("/");
	}

	Response* response = responseNewRedirect("/");
	// Reset cookie
	char* cookie = Cookie_get(req->cookies, "sid");
	if(!cookie)	 // Cookie not found
		goto ret;

	SSPair* entry = container_of_p(cookie, SSPair, value);
	free(entry->value);
	entry->value = "";
	Cookie_av* c_av = cookie_av_new();
	cookie_av_set_expires(c_av, -1);
	concatenate_cookie_av(entry, c_av);
	responseAddCookie(response, entry);
	cookie_av_delete(c_av);
ret:
	return response;
}