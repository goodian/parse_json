#include <stdlib.h>
#include <string.h>

typedef unsigned int size_t;

enum state {
    begin,        //key/value pair begin
    k_begin_s_d,  //begin with "
    k_begin_s_s,  //begin with '
    k_begin_n,    //begin with no " nor '
    k_end,
    v_begin_s_d, //begin with "
    v_begin_s_s, //begin with '
    v_begin_n,   //begin with no " nor '
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
                ret = http_json_arg_decode(session, pos, v_len);
                pos = v + v_len - 1;
                //printf ("\npos is %.*s.\n", v_len, pos);
                //printf ("ret is %d.\n", ret);
                if (ret != 0)
                    goto end_with_error;
            } 
			//
            break;
        case ':':
			if (e_stat == k_begin_n) {
				e_stat = k_end;
				k_len = pos - k;
			}
			//
            break;                                  
        case '\"':                                  
            if (*(pos - 1) == '\\')
                break;
			if (e_stat == begin || e_stat == v_end) {
				e_stat = k_begin_s_d;
				k = pos+ 1;
			} else if (e_stat == k_begin_s_d) {
				e_stat = k_end;
				k_len = pos - k;
			} else if (e_stat == k_end) {
				e_stat = v_begin_s_d;
				v = pos + 1;
			} else if (e_stat == v_begin_s_d) {
				e_stat = v_end;
				v_len = pos - v;
			} 
			break;
			//
        case '\'':
			if (*(pos - 1) == '\\')
				break;
			if (e_stat == begin || e_stat == v_end) {
				e_stat = k_begin_s_s;
				k = pos + 1;
			} else if (e_stat == k_begin_s_s) {
				e_stat = k_end;
				k_len = pos - k;
			} else if (e_stat == k_end) {
				e_stat = v_begin_s_s;
				v = pos + 1;
			} else if (e_stat == v_begin_s_s) {
				e_stat = v_end;
				v_len = pos - v;
			}
			break;
			//
        case '}': //????
            if (e_stat == v_end)
                e_stat = begin;
			else if (e_stat == v_begin_n) {
				e_stat = v_end;	
				v_len = pos - v;
			}
            break;                                  
        case ',':
            //{"a":123}
			if (e_stat == k_begin_n) {
				e_stat = k_end;
				k_len = pos - k;
			} else if (e_stat == v_begin_n) {
				e_stat = v_end;
				v_len = pos - v;
			} else if (e_stat == k_end) {
				break;
			} else if (e_stat == v_end) {
				e_stat = begin;
			}
            break;                                  
        case '[':
		    //printf("==.\n");
            if (e_stat == k_end) {
            	r_bracket = find_right_bracket(pos, e - pos);
                v = pos;
				//printf("==%s.\n", v);
                v_len = r_bracket + 1;
                e_stat = v_end;
                pos = v + v_len - 1;
            }
			break;
        default:
			if (e_stat == begin) {
				e_stat = k_begin_n;
				k = pos;
			} else if (e_stat == k_end) {
				e_stat = v_begin_n;
				v = pos;
			} else if (e_stat == v_end) {
				e_stat = k_begin_n;
				k = pos;
			}
			//
            break;
        } //end switch                                           
	    //printf("pos is %s.\n", pos);
	    //printf("e_stat is %d.\n", e_stat);
        if (e_stat == v_end) {
            printf("key: %.*s.\n", k_len, k);
            printf("value: %.*s.\n", v_len, v);
            //e_stat = begin;
            //插入arglist中
        }
    }                                               
	return 0;

end_with_error:
    printf("return with error, pos is %s\n", pos);
    return -1;
}   

int find_right_bracket(char *pos, const size_t len)
{
    char *e = pos + len;
    char *s = pos;
    size_t i = 0, ret = 0;
    //l记录左括号个数
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
    //l记录左括号个数
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
    char *str = "{\"Name\":\"VIDEO\",\"type\":0,\"lastScene\":-1,\"ID\":5}";
 
    http_json_arg_decode(NULL, str, strlen(str));

    return 0;
}
