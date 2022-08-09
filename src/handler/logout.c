#include "logout.h"
#include "helper.h"

Response *logout(const Request *req)
{
    const Account *my_acc = get_account(req->cookies);
    if (unlikely(!my_acc))  // It's usually logged in
        return responseNewRedirect("/");

    accountDel((Account *) my_acc);

    Response *response = responseNewRedirect("/");

    Cookie *cookie = Cookie_init(eXpire_pair("sid", "", SSPair));
    char buf[64];
    Cookie_gen_expire(buf, -1);
    Cookie_set_attr(cookie, EXPIRE, buf);
    responseAddCookie(response, cookie);  // Copy
    Cookie_delete(cookie);                // delete

    return response;
}