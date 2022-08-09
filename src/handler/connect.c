#include <stdio.h>
#include <string.h>

#include "connect.h"
#include "db.h"
#include "helper.h"

#include "models/connection.h"

Response *connect(const Request *req)
{
    const Account *my_acc = get_account(req->cookies);
    char redir_to[32] = {"/dashboard/"};
    if (unlikely(my_acc == NULL))
        goto fail;

    const Account *account = accountGetById(get_db(), get_id(req->uri));
    if (!account)
        goto fail;

    connectionDel(connectionCreate(get_db(), my_acc->id, account->id));

    snprintf(redir_to, 32, "/profile/%d/", account->id);

    accountDel((Account *) account);

fail:
    accountDel((Account *) my_acc);
    return responseNewRedirect(redir_to);
}
