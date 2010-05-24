/*
 * This file is part of the Hybris programming language interpreter.
 *
 * Copyleft of Simone Margaritelli aka evilsocket <evilsocket@gmail.com>
 *
 * Hybris is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Hybris is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Hybris.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <hybris.h>

HYBRIS_DEFINE_FUNCTION(hurlencode);
HYBRIS_DEFINE_FUNCTION(hurldecode);
HYBRIS_DEFINE_FUNCTION(hbase64encode);
HYBRIS_DEFINE_FUNCTION(hbase64decode);

HYBRIS_EXPORTED_FUNCTIONS() {
    {"urlencode", 	 hurlencode,    H_REQ_ARGC(1), { H_REQ_TYPES(otString) } },
    {"urldecode", 	 hurldecode,    H_REQ_ARGC(1), { H_REQ_TYPES(otString) } },
    {"base64encode", hbase64encode, H_REQ_ARGC(1), { H_REQ_TYPES(otString) } },
    {"base64decode", hbase64decode, H_REQ_ARGC(1), { H_REQ_TYPES(otString) } },
    { "", NULL }
};


INLINE char a2i(char ch) {
	return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

INLINE char i2a(char code) {
	static char hex[] = "0123456789abcdef";
	return hex[code & 15];
}

INLINE bool is_base64(unsigned char c) {
	return (isalnum(c) || (c == '+') || (c == '/'));
}

HYBRIS_DEFINE_FUNCTION(hurlencode){
    Object *_return;

    char *pstr,
         *buf,
         *pbuf;

    vm_parse_argv( "p", &pstr );

    pbuf = buf = (char *)malloc( strlen(pstr) * 3 + 1 );

    while(*pstr){
        if( isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~' ){
            *pbuf++ = *pstr;
        }
        else if( *pstr == ' ' ){
            *pbuf++ = '+';
        }
        else{
            *pbuf++ = '%',
            *pbuf++ = i2a(*pstr >> 4),
            *pbuf++ = i2a(*pstr & 15);
        }
        pstr++;
    }
    *pbuf = '\0';

    _return = ob_dcast( gc_new_string(buf) );

    free(buf);

    return _return;
}

HYBRIS_DEFINE_FUNCTION(hurldecode){
    Object *_return;

    char *pstr,
         *buf,
         *pbuf;

    vm_parse_argv( "p", &pstr );

    pbuf = buf  = (char *)malloc( strlen(pstr) + 1 );

    while(*pstr){
        if(*pstr == '%'){
            if( pstr[1] && pstr[2] ){
                *pbuf++ = a2i(pstr[1]) << 4 | a2i(pstr[2]);
                pstr += 2;
            }
        }
        else if(*pstr == '+'){
            *pbuf++ = ' ';
        }
        else{
            *pbuf++ = *pstr;
        }
        pstr++;
    }

    *pbuf = '\0';

    _return = ob_dcast( gc_new_string(buf) );

    free(buf);

    return _return;
}

HYBRIS_DEFINE_FUNCTION(hbase64encode) {
    static const char b64_charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string ret;
    unsigned char block_3[3];
    unsigned char block_4[4];
    char *str;
    int i = 0,
        j = 0,
        size;

    vm_parse_argv( "p", &str );

    size = strlen(str);

    while( size-- ){
        block_3[i++] = *(str++);
        if( i == 3 ){
            block_4[0] = (block_3[0] & 0xfc) >> 2;
            block_4[1] = ((block_3[0] & 0x03) << 4) + ((block_3[1] & 0xf0) >> 4);
            block_4[2] = ((block_3[1] & 0x0f) << 2) + ((block_3[2] & 0xc0) >> 6);
            block_4[3] = block_3[2] & 0x3f;

            for(i = 0; (i <4) ; i++){
                ret += b64_charset[block_4[i]];
            }
            i = 0;
        }
    }

    if(i){
        for(j = i; j < 3; j++){
            block_3[j] = '\0';
        }
        block_4[0] = (block_3[0] & 0xfc) >> 2;
        block_4[1] = ((block_3[0] & 0x03) << 4) + ((block_3[1] & 0xf0) >> 4);
        block_4[2] = ((block_3[1] & 0x0f) << 2) + ((block_3[2] & 0xc0) >> 6);
        block_4[3] = block_3[2] & 0x3f;

        for(j = 0; (j < i + 1); j++){
            ret += b64_charset[block_4[j]];
        }
        while((i++ < 3)){
            ret += '=';
        }
    }

    return ob_dcast( gc_new_string(ret.c_str()) );
}

HYBRIS_DEFINE_FUNCTION(hbase64decode) {
    static const std::string b64_charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string ret;
    char * str;
    int in_len,
        i = 0,
        j = 0,
        in_ = 0;
    unsigned char block_4[4], block_3[3];

    vm_parse_argv( "p", &str );

    in_len = strlen(str);

    while( in_len-- && ( str[in_] != '=') && is_base64(str[in_]) ){
        block_4[i++] = str[in_];
        in_++;
        if( i == 4 ){
            for( i = 0; i < 4; i++ ){
                block_4[i] = b64_charset.find(block_4[i]);
            }
            block_3[0] = (block_4[0] << 2) + ((block_4[1] & 0x30) >> 4);
            block_3[1] = ((block_4[1] & 0xf) << 4) + ((block_4[2] & 0x3c) >> 2);
            block_3[2] = ((block_4[2] & 0x3) << 6) + block_4[3];

            for( i = 0; (i < 3); i++ ){
                ret += block_3[i];
            }
            i = 0;
        }
    }

    if(i){
        for( j = i; j <4; j++ ){
            block_4[j] = 0;
        }

        for( j = 0; j < 4; j++ ){
            block_4[j] = b64_charset.find(block_4[j]);
        }

        block_3[0] = (block_4[0] << 2) + ((block_4[1] & 0x30) >> 4);
        block_3[1] = ((block_4[1] & 0xf) << 4) + ((block_4[2] & 0x3c) >> 2);
        block_3[2] = ((block_4[2] & 0x3) << 6) + block_4[3];

        for( j = 0; (j < i - 1); j++ ){
            ret += block_3[j];
        }
    }

    return ob_dcast( gc_new_string(ret.c_str()) );
}
