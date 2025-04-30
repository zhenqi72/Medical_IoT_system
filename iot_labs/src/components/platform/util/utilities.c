#include "utilities.h"


char* strrstr(const char* s1, const char* s2)
{
	if(s1 && *s1 && s2 && *s2){
		char* s1_r = (char*)(s1 + strlen(s1) - 1);
		size_t s2_len = strlen(s2);
		while(s1_r >= s1){
			if(0 == strncmp(s1_r, s2, s2_len))
				return s1_r;
			s1_r--;
		}
	}

	return NULL;
}

char* strnchr(const char* s1, char c, size_t n)
{
	if(NULL == s1 || 0 == n || '\0' == c)
		return NULL;

	for(size_t i=0; i<n; i++) {
		if(s1[i] == '\0')
			break;
		if(s1[i] == c)
			return (char*)&s1[i];
	}
	return NULL;
}

int str_rmtail(char *s, int len, char c)
{
	int num = 0;

	if(len == 0)
		len = strlen(s);

	for(int i = len-1; i >= 0; i--) {
		if(s[i] != c)
			break;
		s[i] = '\0';
		num++;
	}

	return num;
}

uint32_t plt_atoun(const char* c, size_t n)
{
	uint32_t dig = 0;
	const char *org = c;
	while(isdigit((int)*c) && (c-org < n) ){
		dig = dig*10 + *c - '0';
		c++;
	}
	return dig;
}

double plt_atofn(const char* c, size_t n)
{
	double val = 0.0;
	const char* c_end;

	if(c == NULL || n == 0)goto finish_exit;

	c_end = c + n;

	while(c < c_end && isspace((int)*c))c++;

	if(c >= c_end)goto finish_exit;

	{
		int flag = 1;
		if(*c == '-')flag = -1;
		if (*c =='+' ||*c == '-')c++;

		while(c < c_end && isdigit((int)*c)){
			val = val*10.0 + (*c - '0');
			c++;
		}

		double power = 1.0;
		if(c < c_end && *c == '.'){
			c++;
			while(c < c_end && isdigit((int)*c)){
				val = val*10.0 + (*c - '0');
				power *= 10.0;
				c++;
			}
		}
		val = (flag * val) / power;
	}

	if(c >= c_end)goto finish_exit;

	if(*c == 'e'|| *c == 'E'){
		int flag = 1;
		int e = 0;
		if(++c < c_end){
			if(*c == '-')flag = -1;
			if (*c =='+' ||*c == '-')c++;
		}

		while(c < c_end && isdigit((int)*c)){
			e = e*10 + (*c - '0');
			c++;
		}

		if(flag == -1){
			while(e-- > 0)val /= 10.0;
		}
		else{
			while(e-- > 0)val *= 10.0;
		}
	}

finish_exit:

	return val;
}
size_t  plt_axtobuf(const char* in, size_t in_size, uint8_t* out, size_t out_size, size_t *in_len)
{
	const char *org_in = in;
	uint8_t *org_out = out;

	while( isxdigit((int)*in) && isxdigit((int)*(in+1)) &&
			(in  - org_in <  in_size) && (out - org_out < out_size)){

		*out = (0x0F & hex_char_value((int)*(in+1))) | (0xF0 & (hex_char_value((int)*in) << 4));

		in += 2; out += 1;
 	}

	if(in_len)
		*in_len = in  - org_in;

	return out - org_out;
}

uint64_t plt_atou64n(const char* c, size_t n)
{
	uint64_t dig = 0;
	const char *org = c;
	while(isdigit((int)*c) && (c-org < n)){
		dig = dig*10 + *c - '0';
		c++;
	}
	return dig;
}

int  snprintf_hex(char *buf, size_t buf_size, const uint8_t *data, size_t len, char style)
{
	unsigned char spliter,uppercase;
	size_t n = 0;

	if (buf_size == 0)
		return 0;

	spliter = style & 0x7F;
	uppercase = (style & 0x80)?1:0;

	if(!isprint(spliter))
		spliter = 0;

	for (size_t i = 0; i < len; i++) {
		if(i < len-1 && spliter)
			n += snprintf(buf+n, buf_size-n, (uppercase? "%02X%c" : "%02x%c"), data[i], spliter);
		else
			n += snprintf(buf+n, buf_size-n, (uppercase? "%02X" : "%02x"), data[i]);
		if (n >= buf_size) break;
	}

	return (n >= buf_size)?(buf_size-1):n;
}

int64_t plt_atos64n(const char* c, size_t n)
{
    if (*c == '-')
        return 0 - (int64_t) plt_atou64n(c + 1, n - 1);
    else
        return (int64_t) plt_atou64n(c, n);
}


int32_t plt_atoin(const char * c, size_t n)
{
    if(*c == '-')
        return 0 - (int32_t) plt_atoun(c + 1, n - 1);
    else
        return (int32_t) plt_atoun(c, n);
}


int plt_u64toa(uint64_t data, char *c)
{
    int num = 0, index;
    char tmp;

    while(data > 0) {
        c[num++] = data%10 + '0';
        data /= 10;
    }
    c[num] = '\0';
    for(index = 0; index < (num >> 1); index++) {
        tmp = c[index];
        c[index] = c[num - 1 - index];
        c[num - 1 - index] = tmp;
    }

    return num;
}

int plt_s64toa(int64_t data, char *c)
{
	if(data < 0){
		c[0] = '-';
		data = -data;
		return (plt_u64toa(data, c+1) + 1);
	}

	return plt_u64toa(data, c);
}

bool str_all_c(uint8_t *str, uint8_t c, size_t len)
{
	for(size_t i=0; i < len; i++){
		if(str[i] != c)
			return false;
	}
	return true;
}

/*
 * Like memcmp, but ignore differences in case. Always returns -1 if different
 */
int memcasecmp( const void *s1, const void *s2, size_t len )
{
    size_t i;
    unsigned char diff;
    const unsigned char *n1 = s1, *n2 = s2;

    for( i = 0; i < len; i++ )
    {
        diff = n1[i] ^ n2[i];

        if( diff == 0 )
            continue;

        if( diff == 32 &&
            ( ( n1[i] >= 'a' && n1[i] <= 'z' ) ||
              ( n1[i] >= 'A' && n1[i] <= 'Z' ) ) )
        {
            continue;
        }

        return( -1 );
    }

    return( 0 );
}