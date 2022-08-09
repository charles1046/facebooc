#include "login.h"
#include "db.h"
#include "helper.h"
#include "template.h"

#include "models/session.h"


Response *login(const Request *req)
{
    const Account *my_acc = get_account(req->cookies);
    if (unlikely(my_acc != NULL)) {  // It's not usually logged in
        accountDel((Account *) my_acc);
        return responseNewRedirect("/dashboard/");
    }

    Response *response = responseNew();
    Template *template = templateNew("templates/login.html");
    responseSetStatus(response, OK);
    templateSet(template, "active", "login");
    templateSet(template, "subtitle", "Login");

    if (req->method == POST) {
        const Basic_string *username =
            Basic_string_init(body_get(req->postBody, "username"));
        const Basic_string *password =
            Basic_string_init(body_get(req->postBody, "password"));

        if (!username->size) {
            invalid(template, "usernameError", "Username is missing!");
        } else {
            templateSet(template, "formUsername", username->data);
        }

        if (!password->size) {
            invalid(template, "passwordError", "Password is missing!");
        }

        bool valid = account_auth(get_db(), username, password);

        if (valid) {
            Session *session =
                sessionCreate(get_db(), username->data, password->data);
            // Username and password are no longer to use
            Basic_string_delete((Basic_string *) username);
            Basic_string_delete((Basic_string *) password);

            if (session) {
                char expire_string[64];
                Cookie_gen_expire(expire_string, 3600 * 24 * 30);

                // Setting cookie
                Cookie *cookie =
                    Cookie_init(eXpire_pair("sid", session->sessionId, SSPair));
                Cookie_set_attr(cookie, EXPIRE, expire_string);
                responseAddCookie(response, cookie);

                sessionDel(session);
                Cookie_delete(cookie);

                responseSetStatus(response, FOUND);
                responseAddHeader(
                    response, eXpire_pair("Location", "/dashboard/", SSPair));
                goto ret;
            } else {
                invalid(template, "usernameError",
                        "Invalid username or password.");
            }
        } else {
            invalid(template, "usernameError", "Invalid username or password.");
        }

        Basic_string_delete((Basic_string *) username);
        Basic_string_delete((Basic_string *) password);
    }

    responseSetBody_move(response, templateRender(template));

ret:
    templateDel(template);
    return response;
}