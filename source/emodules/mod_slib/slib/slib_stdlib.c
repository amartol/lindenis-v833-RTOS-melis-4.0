/*
*********************************************************************************************************
*                                                    MELIS
*                                    the Easy Portable/Player Develop Kits
*                                               Share Libary Module
*
*                                    (c) Copyright 2011-2014, Sunny China
*                                             All Rights Reserved
*
* File    : slib_stdlib.c
* By      : Sunny
* Version : v1.0
* Date    : 2011-3-21
* Descript: shared libary module stdlib functions.
* Update  : date                auther      ver     notes
*           2011-3-21 18:55:23  Sunny       1.0     Create this file.
*********************************************************************************************************
*/
#include "slib_i.h"
__size  __SLIB_strlen(const char  *pstr);

/*
************************************************************************************************************
*             Local Variables
************************************************************************************************************
*/
static const unsigned char _ToUpperTable[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 'A',  'B',  'C',  'D',  'E',  'F',  'G',
    'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',
    'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',
    'X',  'Y',  'Z', 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 'A',  'B',  'C',  'D',  'E',  'F',  'G',
    'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',
    'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',
    'X',  'Y',  'Z', 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
    0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
    0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7,
    0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
    0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7,
    0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
    0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
    0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
    0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};

#define  MASK_LOW4      0xf
#define  NEGATIVE       1
#define  POSITIVE       0
#define  HEX_x          'x'
#define  HEX_X          'X'

/*
**********************************************************************************************************************
*                                               FUNCTION
*
* Description:
*
* Arguments  :
*
* Returns    :
*
* Notes      :
*
**********************************************************************************************************************
*/

static int  get_value(char ch)
{
    if ('0' <= ch  &&  ch <= '9')
    {
        return (ch - '0');
    }
    else if ('a' <= ch  &&  ch <= 'z')
    {
        return (ch - 'a' + 10);
    }
    else if ('A' <= ch  &&  ch <= 'Z')
    {
        return (ch - 'A' + 10);
    }
    else
    {
        return -1;
    }
}

/*
******************************************************************************************************************
*
*Function Name : int_to_string_dec
*
*Description : This function is to convert an 'int' data 'input' to a string in decimalism, and the string
*              converted is in 'str'.
*
*Input : int input : 'int' data to be converted.
*        char * str : the start address of the string storing the converted result.
*
*Output : void
*
*call for :
*
*Others : None at present.
*
*******************************************************************************************************************
*/
void __SLIB_int2str_dec(int input, char *str)
{
    char stack[12];
    char sign_flag = POSITIVE ;      // 'sign_flag indicates wheater 'input' is positive or negative, default
    int i ;                           // value is 'POSITIVE'.
    int j ;

    if (input == 0)
    {
        str[0] = '0';
        str[1] = '\0';                   // 'str' must end with '\0'
        return ;
    }

    if (input < 0)                       // If 'input' is negative, 'input' is assigned to its absolute value.
    {
        sign_flag = NEGATIVE ;
        input = -input ;
    }

    for (i = 0; input > 0; ++i)
    {
        stack[i] = input % 10 + '0';    // characters in reverse order are put in 'stack' .
        input /= 10;
    }                                   // at the end of 'for' loop, 'i' is the number of characters.

    j = 0;

    if (sign_flag == NEGATIVE)
    {
        str[j++] = '-';    // If 'input' is negative, minus sign '-' is placed in the head.
    }

    for (--i  ; i >= 0; --i, ++j)
    {
        str[j] = stack[i];
    }

    str[j] = '\0';                      // 'str' must end with '\0'
}

/*
******************************************************************************************************************
*
*Function Name : Uint_to_string_dec
*
*Description : This function is to convert an 'unsigned int' data 'input' to a string in decimalism, and the
*              string converted is in 'str'.
*
*Input : int input : 'unsigned int' data to be converted.
*        char * str : the start address of the string storing the converted result.
*
*Output : void
*
*call for :
*
*Others : None at present.
*
*******************************************************************************************************************
*/
void __SLIB_uint2str_dec(unsigned int input, char *str)
{
    char stack[11] ;
    int i ;
    int j ;

    if (input == 0)
    {
        str[0] = '0';
        str[1] = '\0';                   // 'str' must end with '\0'
        return ;
    }

    for (i = 0; input > 0; ++i)
    {
        stack[i] = input % 10 + '0';     // characters in reverse order are put in 'stack' .
        input /= 10;
    }                                    // at the end of 'for' loop, 'i' is the number of characters.

    for (--i, j = 0; i >= 0; --i, ++j)
    {
        str[j] = stack[i];
    }

    str[j] = '\0';                       // 'str' must end with '\0'
}

/*
******************************************************************************************************************
*
*Function Name : int_to_string_hex
*
*Description : This function is to convert an 'int' data 'input' to a string in hex, and the string
*              converted is in 'str'.
*
*Input : int input : 'int' data to be converted.
*        char * str : the start address of the string storing the converted result.
*        int hex_flag : hex_flag is just a option to distinguish whether hex format is '0f' or '0F'.
*
*Output : void
*
*call for :
*
*Others : None at present.
*
*******************************************************************************************************************
*/
void __SLIB_int2str_hex(int input, char *str, int hex_flag)
{
    int i;
    int j;
    char stack[8];

    for (i = 0; i < 8; ++i)
    {
        if (hex_flag == HEX_X)
        {
            stack[i] = "0123456789ABCDEF"[input & MASK_LOW4];
        }
        else
        {
            stack[i] = "0123456789abcdef"[input & MASK_LOW4];
        }

        input >>= 4;
    }

    str[0] = '0';
    str[1] = hex_flag;                         // Hex format shoud start with "0x" or "0X".

    for (i = 7, j = 2; i >= 0; --i, ++j)
    {
        str[j] = stack[i];
    }

    str[j] = '\0';                             // 'str' must end with '\0'
}

/*******************************************************************************
*
*             SLIB_atoi
*
  Description:
  FS internal function. Convert string to int. The function stops with
  the first character it cannot convert. It expects decimal numbers only.
  It can handle +/- at the beginning and leading 0. It cannot handle
  HEX or any other numbers.

  Parameters:
  s           - Pointer to a zero terminated string.

  Return value:
  0           - In case of any problem or if the converted value is zero.
  !=0         - Integer value of the converted string.
*/

int __SLIB_atoi(const char *s)
{
    int value;
    int len;
    int i;
    unsigned int base;
    const char *t;
    char c;
    signed char sign;
    value = 0;
    /* Check for +/- */
    sign = 1;
    len = __SLIB_strlen((char *)s);

    if (len <= 0)
    {
        return 0;
    }

    t = s;

    if (*t == '-')
    {
        t++;
        sign = -1;
    }
    else if (*t == '+')
    {
        t++;
    }

    /* Skip leading 0 */
    len = __SLIB_strlen((char *)t);

    if (len <= 0)
    {
        return 0;
    }

    while (1)
    {
        if (*t != '0')
        {
            break;
        }

        t++;
        len--;

        if (len <= 0)
        {
            break;
        }
    }

    /* Find end of number */
    for (i = 0; i < len; i++)
    {
        if (t[i] > '9')
        {
            break;
        }

        if (t[i] < '0')
        {
            break;
        }
    }

    len = i;

    if (len <= 0)
    {
        return 0;
    }

    /* Calculate base */
    base = 1;

    for (i = 1; i < len; i++)
    {
        base *= 10;
    }

    /* Get value */
    for (i = 0; i < len; i++)
    {
        c = t[i];

        if (c > '9')
        {
            break;
        }

        if (c < '0')
        {
            break;
        }

        c -= '0';
        value += c * base;
        base /= 10;
    }

    return sign * value;
}




/*
**********************************************************************************************************************
*                                               eLIBC_toupper
*
* Description: FS internal function. Convert a lowecase letter to a corresponding
*              uppercase letter.
*
* Arguments  : c           - Letter to convert.
*
* Returns    : Corresponding uppercase character.
*
* Notes      :
*
**********************************************************************************************************************
*/
int __SLIB_toupper(int c)
{
    return (int)_ToUpperTable[c];
}

/*
**********************************************************************************************************************
*                                               __SLIB_isspace
*
* Description:
*
* Arguments  :
*
* Returns    :
*
* Notes      :
*
**********************************************************************************************************************
*/
int  __SLIB_isspace(int ch)
{
    if (ch == ' '
        || ch == '\n'
        || ch == '\r'
        || ch == '\t'
        || ch == '\v'
        || ch == '\f')
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/*
**********************************************************************************************************************
*                                               SLIB_strtol
*
* Description:
*
* Arguments  :
*
* Returns    :
*
* Notes      :
*
**********************************************************************************************************************
*/
long __SLIB_strtol(const char *str, const char **err, int base)
{
#define POS    1
#define NEG    0
    const char   *p;
    const char   *q;
    long          sign_label;
    int           value;
    long          sum;
    unsigned long unsum;

    if (str == NULL || base < 0 || base > 36)
    {
        goto error;
    }

    /* skip space character */
    p = str;

    while (__SLIB_isspace(*p))
    {
        p++;
    }

    /* get signed label */
    if (*p == '-')
    {
        p++;
        sign_label = NEG;
    }
    else if (*p == '+')
    {
        p++;
        sign_label = POS;
    }
    else
    {
        sign_label = POS;
    }

    /* get base and first valid number char */
    if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X'))        // hex
    {
        if (base == 0)
        {
            base = 16;
            p += 2;
        }
    }
    else if (p[0] == '0')                                   // oct
    {
        if (base == 0)
        {
            base = 8;
        }
    }
    else                                                    // dec
    {
        if (base == 0)
        {
            base = 10;
        }
    }

    q = p;
    unsum = 0;

    while (1)
    {
        value = get_value(*p);

        if (value < 0 || value >= base)
        {
            break;
        }

        unsum = unsum * base + value;
        p++;
    }

    if (p == q)
    {
        goto error;
    }
    else
    {
        if (err != NULL)
        {
            *err = p;
        }

        if (sign_label == NEG)
        {
            sum = -(long)unsum;
        }
        else
        {
            sum = (long)unsum;
        }

        return sum;
    }

error:

    if (err != NULL)
    {
        *err = str;
    }

    return 0;
}

/*******************************************************************************
*
*             SLIB_atof
*
  Description:
  FS internal function. Convert string to int. The function stops with
  the first character it cannot convert. It expects decimal numbers only.
  It can handle +/- at the beginning and leading 0. It cannot handle
  HEX or any other numbers.

  Parameters:
  s           - Pointer to a zero terminated string.

  Return value:
  0           - In case of any problem or if the converted value is zero.
  !=0         - float value of the converted string.
*/
int __isdigit(int cChar)
{
    // Return true if the character is between 0 and 9 inclusive.
    if ((cChar >= '0') && (cChar <= '9'))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

double __SLIB_atof(const char *nptr)
{
    double retval = 0.0;
    double result;
    const char *p;
    int shift = 0, sign = 1, exp_sign = 1, exp = 0;
    p = nptr;

    while (*p)
    {
        if (*p == ' ' || *p == '\t')
        {
            p++;
            continue;
        }

        break;
    }

    /**
     * '+'/'-'判断符号位: 不能两个连续的'+'或'-'否则函数立即返回
     */
    if (*p == '+')
    {
        sign = 1;
        ++p;
    }
    else if (*p == '-')
    {
        sign = -1;
        ++p;
    }
    else
    {
        ;
    }

    // int_part:
    while (*p)
    {
        if (__isdigit(*p))
        {
            retval = (double)((int)retval * 10 + (*p - '0'));
        }
        else if (*p == '.')
        {
            ++p;
            goto non_int_part;
        }
        else if (*p == 'e' || *p == 'E')
        {
            ++p;
            goto exp_part;
        }
        else
        {
            return retval;
        }

        ++p;
    }

non_int_part:

    while (*p)
    {
        ++shift;

        if (p - nptr > 100)
        {
            break;
        }

        if (__isdigit(*p))
        {
            int f = shift, f1 = (*p - '0');
            double f2 = (double)f1;

            while (f--)
            {
                f2 /= 10;
            }

            retval += f2;
        }
        else if (*p == 'e' || *p == 'E')
        {
            ++p;
            goto exp_part;
        }
        else
        {
            return retval;
        }

        ++p;
    }

exp_part:

    /**
     * 判断符号位: 但不允许同时出现两个符号
     * 否则函数立即返回
     */
    if (*p == '-')
    {
        exp_sign = -1;
        ++p;
    }
    else if (*p == '+')
    {
        exp_sign = 1;
        ++p;
    }
    else
    {
        ;
    }

    while (*p)
    {
        if (__isdigit(*p))
        {
            exp = exp * 10 + (*p - '0');
        }
        else
        {
            break;
        }

        ++p;
    }

    if (exp_sign > 0)
    {
        while (exp)
        {
            retval *= 10;
            exp--;
        }
    }
    else
    {
        while (exp)
        {
            retval /= 10;
            exp--;
        }
    }

    result = retval * sign;
    return (double)(result);
}

#define isxdigit(c)          (('0' <= (c) && (c) <= '9') || ('a' <= (c) && (c) <= 'f') || ('A' <= (c) && (c) <= 'F'))

static int tolower(int c)
{
    int r = c;

    if (r >= 'A' && r <= 'Z')
    {
        r = r + 0x20;
    }

    return r;
}

static unsigned long __SLIB_strtoul(const char *cp, char **endp, unsigned int base)
{
    unsigned long result = 0, value;

    if (!base)
    {
        base = 10;

        if (*cp == '0')
        {
            base = 8;
            cp++;

            if ((tolower(*cp) == 'x') && isxdigit(cp[1]))
            {
                cp++;
                base = 16;
            }
        }
    }
    else if (base == 16)
    {
        if (cp[0] == '0' && tolower(cp[1]) == 'x')
        {
            cp += 2;
        }
    }

    while (isxdigit(*cp) && (value = __isdigit(*cp) ? *cp - '0' : tolower(*cp) - 'a' + 10) < base)
    {
        result = result * base + value;
        cp++;
    }

    if (endp)
    {
        *endp = (char *)cp;
    }

    return result;
}

static int skip_atoi(const char **s)
{
    int i = 0;

    while (__isdigit(**s))
    {
        i = i * 10 + *((*s)++) - '0';
    }

    return i;
}

static int _vsscanf(const char *buf, const char *fmt, va_list args)
{
    const char *str = buf;
    char *next;
    char digit;
    int num = 0;
    int qualifier;
    int base;
    int field_width;
    int is_sign = 0;

    while (*fmt && *str)
    {
        /* skip any white space in format */
        /* white space in format matchs any amount of
         * white space, including none, in the input.
         */
        if (__SLIB_isspace(*fmt))
        {
            while (__SLIB_isspace(*fmt))
            {
                ++fmt;
            }

            while (__SLIB_isspace(*str))
            {
                ++str;
            }
        }

        /* anything that is not a conversion must match exactly */
        if (*fmt != '%' && *fmt)
        {
            if (*fmt++ != *str++)
            {
                break;
            }

            continue;
        }

        if (!*fmt)
        {
            break;
        }

        ++fmt;

        /* skip this conversion.
         * advance both strings to next white space
         */
        if (*fmt == '*')
        {
            while (!__SLIB_isspace(*fmt) && *fmt)
            {
                fmt++;
            }

            while (!__SLIB_isspace(*str) && *str)
            {
                str++;
            }

            continue;
        }

        /* get field width */
        field_width = -1;

        if (__isdigit(*fmt))
        {
            field_width = skip_atoi(&fmt);
        }

        /* get conversion qualifier */
        qualifier = -1;

        if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L' ||
            *fmt == 'Z' || *fmt == 'z')
        {
            qualifier = *fmt++;

            if (qualifier == *fmt)
            {
                if (qualifier == 'h')
                {
                    qualifier = 'H';
                    fmt++;
                }
                else if (qualifier == 'l')
                {
                    qualifier = 'L';
                    fmt++;
                }
            }
        }

        base = 10;
        is_sign = 0;

        if (!*fmt || !*str)
        {
            break;
        }

        switch (*fmt++)
        {
            case 'c':
            {
                char *s = (char *) va_arg(args, char *);

                if (field_width == -1)
                {
                    field_width = 1;
                }

                do
                {
                    *s++ = *str++;
                } while (--field_width > 0 && *str);

                num++;
            }

            continue;

            case 's':
            {
                char *s = (char *) va_arg(args, char *);

                if (field_width == -1)
                {
                    field_width = 0x7FFFFFFF;
                }

                /* first, skip leading white space in buffer */
                while (__SLIB_isspace(*str))
                {
                    str++;
                }

                /* now copy until next white space */
                while (*str && !__SLIB_isspace(*str) && field_width--)
                {
                    *s++ = *str++;
                }

                *s = '\0';
                num++;
            }

            continue;

            case 'n':
                /* return number of characters read so far */
            {
                int *i = (int *)va_arg(args, int *);
                *i = str - buf;
            }

            continue;

            case 'o':
                base = 8;
                break;

            case 'x':
            case 'X':
                base = 16;
                break;

            case 'i':
                base = 0;

            case 'd':
                is_sign = 1;

            case 'u':
                break;

            case '%':

                /* looking for '%' in str */
                if (*str++ != '%')
                {
                    return num;
                }

                continue;

            default:
                /* invalid format; stop here */
                return num;
        }

        /* have some sort of integer conversion.
         * first, skip white space in buffer.
         */
        while (__SLIB_isspace(*str))
        {
            str++;
        }

        digit = *str;

        if (is_sign && digit == '-')
        {
            digit = *(str + 1);
        }

        if (!digit
            || (base == 16 && !isxdigit(digit))
            || (base == 10 && !__isdigit(digit))
            || (base == 8 && (!__isdigit(digit) || digit > '7'))
            || (base == 0 && !__isdigit(digit)))
        {
            break;
        }

        switch (qualifier)
        {
            case 'H':   /* that's 'hh' in format */
                if (is_sign)
                {
                    signed char *s = (signed char *) va_arg(args, signed char *);
                    *s = (signed char) __SLIB_strtol(str, &next, base);
                }
                else
                {
                    unsigned char *s = (unsigned char *) va_arg(args, unsigned char *);
                    *s = (unsigned char) __SLIB_strtoul(str, &next, base);
                }

                break;

            case 'h':
                if (is_sign)
                {
                    short *s = (short *) va_arg(args, short *);
                    *s = (short) __SLIB_strtol(str, &next, base);
                }
                else
                {
                    unsigned short *s = (unsigned short *) va_arg(args, unsigned short *);
                    *s = (unsigned short) __SLIB_strtoul(str, &next, base);
                }

                break;

            case 'l':
                if (is_sign)
                {
                    long *l = (long *) va_arg(args, long *);
                    *l = __SLIB_strtol(str, &next, base);
                }
                else
                {
                    unsigned long *l = (unsigned long *) va_arg(args, unsigned long *);
                    *l = __SLIB_strtoul(str, &next, base);
                }

                break;

            case 'Z':
            case 'z':
            {
                size_t *s = (size_t *) va_arg(args, size_t *);
                *s = (size_t) __SLIB_strtoul(str, &next, base);
            }
            break;

            default:
                if (is_sign)
                {
                    int *i = (int *) va_arg(args, int *);
                    *i = (int) __SLIB_strtol(str, &next, base);
                }
                else
                {
                    unsigned int *i = (unsigned int *) va_arg(args, unsigned int *);
                    *i = (unsigned int) __SLIB_strtoul(str, &next, base);
                }

                break;
        }

        num++;

        if (!next)
        {
            break;
        }

        str = next;
    }

    /*
     * Now we've come all the way through so either the input string or the
     * format ended. In the former case, there can be a %n at the current
     * position in the format that needs to be filled.
     */
    if (*fmt == '%' && *(fmt + 1) == 'n')
    {
        int *p = (int *)va_arg(args, int *);
        *p = str - buf;
    }

    return num;
}

__s8 __SLIB_sscanf(const char *buf, const char *fmt, ...)
{
    va_list args;
    __s8 i;
    va_start(args, fmt);
    i = _vsscanf(buf, fmt, args);
    va_end(args);
    return i;
}
