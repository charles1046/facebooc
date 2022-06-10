#include "handler/handlers/logout.h"

#include "handler/handler_utility.h"
#include "models/account.h"
#include "utility.h"

Response* logout(Request* req) {
	const Account* my_acc = get_account(req->cookies);
	if(unlikely(!my_acc))  // It's usually logged in
		return responseNewRedirect("/");

	accountDel((Account*)my_acc);

	Response* response = responseNewRedirect("/");

	Cookies* cookie = Cookies_init("sid", "");
	Cookies_set_expire(cookie, "sid", -1);

	responseAddCookie(response, cookie);  // Copy
	Cookies_delete(cookie);				  // delete

	return response;
}