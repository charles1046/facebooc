#include "signup.h"
#include "db.h"
#include "helper.h"
#include "template.h"

#include <string.h>

Response *signup(const Request *req)
{
    const Account *my_acc = get_account(req->cookies);
    if (unlikely(my_acc != NULL)) {
        // If you already logged in, you should not register a new account
        accountDel((Account *) my_acc);
        return responseNewRedirect("/dashboard/");
    }

    Response *response = responseNew();
    Template *template = templateNew("templates/signup.html");
    templateSet(template, "active", "signup");
    templateSet(template, "subtitle", "Sign Up");
    responseSetStatus(response, OK);

    if (req->method == POST) {
        bool valid = false;
        const Basic_string *name =
            Basic_string_init(body_get(req->postBody, "name"));
        const Basic_string *email =
            Basic_string_init(body_get(req->postBody, "email"));
        const Basic_string *username =
            Basic_string_init(body_get(req->postBody, "username"));
        const Basic_string *password =
            Basic_string_init(body_get(req->postBody, "password"));

        // Although confirmPassword is not used in register model, we also use
        // Basic_string for consistency
        const Basic_string *confirmPassword =
            Basic_string_init(body_get(req->postBody, "confirm-password"));

        if (!name->size) {
            invalid(template, "nameError", "You must enter your name!");
        } else if (name->size < 5 || name->size > 50) {
            invalid(template, "nameError",
                    "Your name must be between 5 and 50 characters long.");
        } else {
            templateSet(template, "formName", name->data);
            valid = true;
        }

        if (!email->size) {
            invalid(template, "emailError", "You must enter an email!");
        } else if (strchr(email->data, '@') == NULL) {
            invalid(template, "emailError", "Invalid email.");
        } else if (email->size < 3 || email->size > 50) {
            invalid(template, "emailError",
                    "Your email must be between 3 and 50 characters long.");
        } else if (!accountCheckEmail(get_db(), email)) {
            invalid(template, "emailError", "This email is taken.");
        } else {
            templateSet(template, "formEmail", email->data);
            valid = true;
        }

        if (!username->size) {
            invalid(template, "usernameError", "You must enter a username!");
        } else if (username->size < 3 || username->size > 50) {
            invalid(template, "usernameError",
                    "Your username must be between 3 and 50 characters long.");
        } else if (!accountCheckUsername(get_db(), username)) {
            invalid(template, "usernameError", "This username is taken.");
        } else {
            templateSet(template, "formUsername", username->data);
            valid = true;
        }

        if (!password->size) {
            invalid(template, "passwordError", "You must enter a password!");
        } else if (password->size < 8) {
            invalid(template, "passwordError",
                    "Your password must be at least 8 characters long!");
        } else
            valid = true;

        if (!confirmPassword->size) {
            invalid(template, "confirmPasswordError",
                    "You must confirm your password.");
        } else if (strcmp(password->data, confirmPassword->data) != 0) {
            invalid(template, "confirmPasswordError",
                    "The two passwords must be the same.");
        } else
            valid = true;

        if (valid) {
            // Register an account
            accountDel(
                accountCreate(get_db(), name, email, username, password));
            Basic_string_delete((Basic_string *) name);
            Basic_string_delete((Basic_string *) email);
            Basic_string_delete((Basic_string *) username);
            Basic_string_delete((Basic_string *) password);
            Basic_string_delete((Basic_string *) confirmPassword);

            responseSetStatus(response, FOUND);
            responseAddHeader(response,
                              eXpire_pair("Location", "/login/", SSPair));
            goto ret;
        }
    }

    responseSetBody_move(response, templateRender(template));
ret:
    templateDel(template);
    return response;
}
