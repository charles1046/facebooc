#include "handler/handlers/signup.h"

#include <string.h>

#include "db.h"
#include "handler/handler_utility.h"
#include "models/account.h"
#include "models/like.h"
#include "utility.h"

Response* signup(Request* req) {
	const Account* my_acc = get_account(req->cookies);
	if(unlikely(my_acc != NULL)) {
		// If you already logged in, you should not register
		accountDel((Account*)my_acc);
		return responseNewRedirect("/dashboard/");
	}
	Response* response = responseNew();
	Template* template = templateNew("templates/signup.html");
	templateSet(template, "active", "signup");
	templateSet(template, "subtitle", "Sign Up");
	responseSetStatus(response, OK);
	if(req->method == POST) {
		bool valid = true;
		const char* name = body_get(req->postBody, "name");
		const char* email = body_get(req->postBody, "email");
		const char* username = body_get(req->postBody, "username");
		const char* password = body_get(req->postBody, "password");
		const char* confirmPassword = body_get(req->postBody, "confirm-password");
		if(!name) {
			template_set_error_message(template, "nameError", "You must enter your name!");
		}
		else if(strlen(name) < 5 || strlen(name) > 50) {
			template_set_error_message(template, "nameError",
									   "Your name must be between 5 and 50 characters long.");
		}
		else {
			templateSet(template, "formName", name);
		}
		if(!email) {
			template_set_error_message(template, "emailError", "You must enter an email!");
		}
		else if(strchr(email, '@') == NULL) {
			template_set_error_message(template, "emailError", "Invalid email.");
		}
		else if(strlen(email) < 3 || strlen(email) > 50) {
			template_set_error_message(template, "emailError",
									   "Your email must be between 3 and 50 characters long.");
		}
		else if(!accountCheckEmail(get_db(), email)) {
			template_set_error_message(template, "emailError", "This email is taken.");
		}
		else {
			templateSet(template, "formEmail", email);
		}
		if(!username) {
			template_set_error_message(template, "usernameError", "You must enter a username!");
		}
		else if(strlen(username) < 3 || strlen(username) > 50) {
			template_set_error_message(template, "usernameError",
									   "Your username must be between 3 and 50 characters long.");
		}
		else if(!accountCheckUsername(get_db(), username)) {
			template_set_error_message(template, "usernameError", "This username is taken.");
		}
		else {
			templateSet(template, "formUsername", username);
		}
		if(!password) {
			template_set_error_message(template, "passwordError", "You must enter a password!");
		}
		else if(strlen(password) < 8) {
			template_set_error_message(template, "passwordError",
									   "Your password must be at least 8 characters long!");
		}
		if(!confirmPassword) {
			template_set_error_message(template, "confirmPasswordError",
									   "You must confirm your password.");
		}
		else if(strcmp(password, confirmPassword) != 0) {
			template_set_error_message(template, "confirmPasswordError",
									   "The two passwords must be the same.");
		}
		if(valid) {
			Account* account = accountCreate(get_db(), name, email, username, password);
			if(account) {
				accountDel(account);

				responseSetStatus(response, FOUND);
				responseAddHeader(response, eXpire_pair("Location", "/login/", SSPair));
				goto ret;
			}
			else {
				template_set_error_message(template, "nameError",
										   "Unexpected error. Please try again later.");
			}
		}
	}
	responseSetBody(response, templateRender(template));
ret:
	templateDel(template);
	return response;
}