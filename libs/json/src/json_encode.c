/**
 * Copyright (c) 2015 Runtime Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include <stdio.h>
#include <string.h>

#include <json/json.h>

#define JSON_ENCODE_OBJECT_START(__e) \
    (__e)->je_write((__e)->je_arg, "{", sizeof("{")-1);

#define JSON_ENCODE_OBJECT_END(__e) \
    (__e)->je_write((__e)->je_arg, "}", sizeof("}")-1);

#define JSON_ENCODE_ARRAY_START(__e) \
    (__e)->je_write((__e)->je_arg, "[", sizeof("[")-1);

#define JSON_ENCODE_ARRAY_END(__e) \
    (__e)->je_write((__e)->je_arg, "]", sizeof("]")-1);


int 
json_encode_object_start(struct json_encoder *encoder, void *buf, 
        json_write_func_t wf)
{
    encoder->je_write = wf;
    encoder->je_arg = buf;

    JSON_ENCODE_OBJECT_START(encoder);

    return (0);
}

static int 
json_encode_value(struct json_encoder *encoder, struct json_value *jv)
{
    int rc;
    int i;
    int len;

    switch (jv->jv_type) {
        case JSON_VALUE_TYPE_BOOL:
            len = sprintf(encoder->je_encode_buf, "%s", 
                    jv->jv_val.u > 0 ? "true" : "false");
            encoder->je_write(encoder->je_arg, encoder->je_encode_buf, len);
            break;
        case JSON_VALUE_TYPE_UINT64:
            len = sprintf(encoder->je_encode_buf, "%llu", jv->jv_val.u);
            encoder->je_write(encoder->je_arg, encoder->je_encode_buf, len);
            break;
        case JSON_VALUE_TYPE_INT64:
            len = sprintf(encoder->je_encode_buf, "%lld", 
                    (int64_t) jv->jv_val.u);
            encoder->je_write(encoder->je_arg, encoder->je_encode_buf, len);
            break;
        case JSON_VALUE_TYPE_STRING:
            encoder->je_write(encoder->je_arg, jv->jv_val.str, jv->jv_len);
            break;
        case JSON_VALUE_TYPE_ARRAY:
            JSON_ENCODE_ARRAY_START(encoder);
            for (i = 0; i < jv->jv_len; i++) {
                rc = json_encode_value(encoder, jv->jv_val.composite.values[i]);
                if (rc != 0) {
                    goto err;
                }
                encoder->je_write(encoder->je_arg, ",", sizeof(",")-1);
            }
            JSON_ENCODE_ARRAY_END(encoder);
            break;
        case JSON_VALUE_TYPE_OBJECT:
            JSON_ENCODE_OBJECT_START(encoder);
            for (i = 0; i < jv->jv_len; i++) {
                rc = json_encode_object_entry(encoder, 
                        jv->jv_val.composite.keys[i], 
                        jv->jv_val.composite.values[i]);
                if (rc != 0) {
                    goto err;
                }
            }
            JSON_ENCODE_OBJECT_END(encoder);
            break;
        default:
            rc = -1;
            goto err;
    }


    return (0);
err:
    return (rc);
}


int 
json_encode_object_entry(struct json_encoder *encoder, char *key, 
        struct json_value *val)
{
    int rc;

    /* Write the key entry */
    encoder->je_write(encoder->je_arg, "\"", sizeof("\"")-1);
    encoder->je_write(encoder->je_arg, key, strlen(key));
    encoder->je_write(encoder->je_arg, "\": ", sizeof("\": ")-1);

    rc = json_encode_value(encoder, val);
    if (rc != 0) {
        goto err;
    }
    encoder->je_write(encoder->je_arg, ",", sizeof(",")-1);

    return (0);
err:
    return (rc);
}

int 
json_encode_object_finish(struct json_encoder *encoder)
{
    JSON_ENCODE_OBJECT_END(encoder);

    return (0);
}