#include <stdlib.h>
#include <string.h>

typedef unsigned int size_t;

enum state {
    begin, 
    k_begin,
    k_end,
    v_begin_s, //以引号开始的alue
    v_begin_n, //以数字形式开始的alue
    v_end,
    done
};

//static inline int http_json_arg_decode(http_session_t *session, char *s, const size_t len)
static inline int http_json_arg_decode(char *session, char *s, const size_t len)
{   
    int k_len, v_len;
    char *k = NULL, *v = NULL, *e = s + len;        
    char *pos = s;                                  
    int ret = 0;
    size_t r_braces = 0, r_bracket = 0;

    enum state e_stat = begin;
    
    while (s < e && *s != '{')                      
        s++;                                        

    for (; pos < e; pos++) {                        
        switch (*pos) {                              
        case ' ':
            break;
        case '{':
            //if (e_stat == v_begin) {
            if (e_stat == k_end) {
                v = pos;
                //找到与之匹配的右括号
                r_braces = find_right_braces(pos, e - pos);
                v_len = r_braces + 1;
                e_stat = v_end;
                ret = http_json_arg_decode(session, pos, r_braces);
                pos = v + v_len - 1;
                printf ("ret is %d.\n", ret);
                if (ret != 0)
                    goto end_with_error;
            }
            break;
        case ':':
            if (e_stat == begin || e_stat == k_begin || e_stat == v_end)
                goto end_with_error;
            break;                                  
        case '\"':                                  
            if (e_stat == begin || e_stat == v_end) {
                e_stat = k_begin;
                k = pos + 1;
            } else if (e_stat == k_begin) {
                k_len = pos - k;
                e_stat = k_end;
            } else if (e_stat == k_end) {
                v = pos + 1;
                e_stat = v_begin_s;
            } else if (e_stat == v_begin_s) {
                v_len = pos - v;
                e_stat = v_end;
            } //else if (e_stat == v_end) 
                //goto end_with_error;
            break;                                  
        case '}':
            if (e_stat == v_end)
                e_stat = done;
            else if (e_stat == v_begin_n) {
                v_len = pos - v;
                e_stat = v_end;
            }
            break;                                  
        case ',':
            //{"a":123}
            if (e_stat == v_begin_n) {
                v_len = pos - v;
                e_stat = v_end;
            }
            if (e_stat != v_end && e_stat != begin)
                goto end_with_error;
            break;                                  
        case '[':
            if (e_stat == k_end) {
            	r_bracket = find_right_bracket(pos, e - pos);
                v = pos;
                v_len = r_bracket + 1;
                e_stat = v_end;
                pos = v + v_len - 1;
            }
        default:
            if (e_stat == k_end) {
                v = pos;
                e_stat = v_begin_n;
            }
            break;
        } //end switch                                           
        if (e_stat == done)
            break;
        else if (e_stat == v_end) {
            printf("key: %.*s.\n", k_len, k);
            printf("value: %.*s.\n", v_len, v);
            e_stat = begin;
            //插入arglist中
        }
    }                                               

    return 0;
end_with_error:
    return -1;
}   

int find_right_bracket(char *pos, const size_t len)
{
    char *e = pos + len;
    char *s = pos;
    size_t i = 0, ret = 0;
    //l 和 r分别记录左括号和右括号的个数
    size_t l = 0;

    if (*pos != '[')
        return -1;

    for (; pos < e; pos++) {
        if (*pos != '[' && *pos != ']')
            continue;

        if (*pos == '[')
            l++;
        else if (*pos == ']') {
            if (l == 1)
                return pos - s;
            l--;
        }
    }

    return -1;
}

int find_right_braces(char *pos, const size_t len)
{
    char *e = pos + len;
    char *s = pos;
    size_t i = 0, ret = 0;
    //l 和 r分别记录左括号和右括号的个数
    size_t l = 0;

    if (*pos != '{')
        return -1;

    for (; pos < e; pos++) {
        if (*pos != '{' && *pos != '}')
            continue;

        if (*pos == '{')
            l++;
        else if (*pos == '}') {
            if (l == 1)
                return pos - s;
            l--;
        }
    }

    return -1;
}

int main()
{
    //char *s = "{\"Name\":\"VIDEO\",\"type\":0,\"lastScene\":-1,\"ID\":5}";
    char *s= "{\"e\" : [\"xiaoao\",\"xiaoba\",\"xiaoma\"],\"Education\":{\"GradeSchool\" : \"huayixiao\",\"MiddleSchool\" : [\"l\" , \"gao\"], \"University\" :{ \"Name\" : \"aa\", \"Specialty\" : [\"d\",\"aaa\"]}}}";
    //char *s = "{\"N\":\"V\",\"t\":\"0\",\"lastScene\":\"-1\",\"ID\":\"5\"}";
    //char *s = "{\"t\":0,\"l\":-1,\"ID\":5}";
    printf(s);
    http_json_arg_decode(s, s, strlen(s));
    return 0;
}
