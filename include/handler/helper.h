#ifndef __HANDLER_HELPER__
#define __HANDLER_HELPER__

#include "http/cookies.h"
#include "models/account.h"

// Get sid string from cookies
const Account *get_account(const Cookies *req);

// format: "/%s/<id>/"
int get_id(const char *uri);

typedef struct Template Template;
int invalid(Template *template, const char *key, const char *value);

#endif