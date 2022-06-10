#include "handler/handlers/login.h"

#include "db.h"
#include "handler/handler_utility.h"
#include "http/cookies.h"
#include "models/account.h"
#include "models/session.h"
#include "utility.h"

Response* login(Request* req) {
	const Account* my_acc = get_account(req->cookies);
	if(unlikely(my_acc != NULL)) {	// It's not usually logged in
		accountDel((Account*)my_acc);
		return responseNewRedirect("/dashboard/");
	}
	Response* response = responseNew();
	Template* template = templateNew("templates/login.html");
	responseSetStatus(response, OK);
	templateSet(template, "active", "login");
	templateSet(template, "subtitle", "Login");
	if(req->method == POST) {
		const char* username = body_get(req->postBody, "username");
		const char* password = body_get(req->postBody, "password");
		if(!username) {
			template_set_error_message(template, "usernameError", "Username missing!");
		}
		else {
			templateSet(template, "formUsername", username);
		}
		if(!password) {
			template_set_error_message(template, "passwordError", "Password missing!");
		}
		bool valid = account_auth(get_db(), username, password);
		if(valid) {
			Session* session = sessionCreate(get_db(), username, password);
			if(session) {
				// Setting cookie
				Cookies* cookie = Cookies_init("sid", session->sessionId);
				Cookies_set_expire(cookie, "sid", 3600 * 24 * 30);
				responseAddCookie(response, cookie);

				sessionDel(session);
				Cookies_delete(cookie);

				responseSetStatus(response, FOUND);
				responseAddHeader(response, eXpire_pair("Location", "/dashboard/", SSPair));
				goto ret;
			}
			else {
				template_set_error_message(template, "usernameError",
										   "Invalid username or password.");
			}
		}
		else {
			template_set_error_message(template, "usernameError", "Invalid username or password.");
		}
	}
	responseSetBody(response, templateRender(template));
ret:
	templateDel(template);
	return response;
}