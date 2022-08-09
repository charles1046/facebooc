#include "helper.h"
#include "template.h"

#include <db.h>
#include <string.h>

const Account *get_account(const Cookies *c)
{
    // TODO: Use an object pool to reduce malloc times
    if (unlikely(c == NULL))
        return NULL;
    Cookie *cookie = Cookies_get(c, "sid");
    const Basic_string sid = {.data = (char *) Cookie_get_attr(cookie, VALUE),
                              .size = strlen(Cookie_get_attr(cookie, VALUE))};
    return accountGetBySId(get_db(), &sid);
}

int get_id(const char *uri)
{
    char *begin = strchr(uri + 1, '/') + 1;
    char *end = strchr(begin, '/');
    const size_t len = end - begin;

    char new_str[10] = {0};
    memcpy(new_str, begin, len);

    return atoi(new_str);
}

int invalid(Template *template, const char *key, const char *value)
{
    templateSet(template, key, value);
    return false;
}