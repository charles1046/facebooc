#include "handler/handlers/login.h"

#include <string.h>

#include "db.h"
#include "handler/handler_utility.h"
#include "models/account.h"
#include "models/session.h"
#include "utility.h"

Response* login(Request* request) {
	const Account* my_acc = get_account(request->cookies);
	if(unlikely(my_acc != NULL)) {	// It's not usually logged in
		accountDel((Account*)my_acc);
		return responseNewRedirect("/dashboard/");
	}

	Response* response = responseNew();
	Template* template = templateNew("templates/login.html");
	responseSetStatus(response, OK);
	templateSet(template, "active", "login");
	templateSet(template, "subtitle", "Login");

	if(request->method == POST) {
		const char* username = body_get(request->postBody, "username");
		const char* password = body_get(request->postBody, "password");

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
				responseSetStatus(response, FOUND);

				// Setting cookie
				SSPair* cookie = malloc(sizeof(SSPair));
				cookie->key = strdup("sid");
				cookie->value = strdup(session->sessionId);
				sessionDel(session);

				Cookie_av* c_av = cookie_av_new();
				cookie_av_set_expires(c_av, 3600 * 24 * 30);

				// Concatenate
				concatenate_cookie_av(cookie, c_av);
				// Move cookie into
				responseAddHeader_move(response, cookie);
				cookie_av_delete(c_av);

				responseAddHeader(response, &(SSPair){ .key = "Location", .value = "/dashboard/" });
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