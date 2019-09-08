#include <wchar.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
 
struct putchdata {
    wchar_t *buffer;
    size_t maxlen, count;
};
 
static void reverse(char *str, size_t len) {
    if (len > 1) {
        char *start = str;
        char *end = str + len - 1;
        char tmp;
 
        while (start < end) {
            tmp = *end;
            *end-- = *start;
            *start++ = tmp;
        }
    }
}
 
static size_t itoa(unsigned num, char *dst, unsigned base, int issigned, int addplus, int uppercase) {
    int a = uppercase ? 'A' : 'a';
    int negative = 0;
    char *d = dst;
    size_t len;
 
    if (num == 0) {
        *d = '0';
        return 1;
    }
 
    if (issigned && (int)num < 0 && base == 10) {
        negative = 1;
        num = -num;
    }
 
    while (num > 0) {
        unsigned rem = num % base;
        num /= base;
        *d++ = (rem > 9) ? (rem - 10 + a) : (rem + '0');
    }
 
    if (negative)
        *d++ = '-';
    else if (addplus)
        *d++ = '+';
 
    len = d - dst;
    reverse(dst, len);
 
    return len;
}
 
static size_t lltoa(unsigned long long num, char *dst, unsigned base, int issigned, int addplus, int uppercase) {
    int a = uppercase ? 'A' : 'a';
    int negative = 0;
    char *d = dst;
    size_t len;
 
    if (num == 0) {
        *d = '0';
        return 1;
    }
 
    if (issigned && (signed long long)num < 0 && base == 10) {
        negative = 1;
        num = -num;
    }
 
    while (num > 0) {
        unsigned rem = num % base;
        num /= base;
        *d++ = (rem > 9) ? (rem - 10 + a) : (rem + '0');
    }
 
    if (negative)
        *d++ = '-';
    else if (addplus)
        *d++ = '+';
 
    len = d - dst;
    reverse(dst, len);
 
    return len;
}
 
static void putchproc(wchar_t ch, struct putchdata *pcd) {
    size_t index = pcd->count;
 
    if (ch != '\0')
        pcd->count++;
 
    if (pcd->count > pcd->maxlen)
        return;
 
    if (pcd->count == pcd->maxlen) {
        pcd->buffer[index] =  '\0';
        return;
    }
 
    pcd->buffer[index] = ch;
}
 
#define PUTCH(ch) putchproc((ch), &pcd)
 
int vswprintf(wchar_t *buffer, size_t maxlen, const wchar_t *fmt, va_list ap) {
    struct _reent *r = _REENT;
    struct putchdata pcd;
    wchar_t ch;
    const wchar_t *src;
    char tmp[128];
    const char *hsrc;
 
    pcd.buffer = buffer;
    pcd.maxlen = maxlen;
    pcd.count = 0;
 
    while ((ch = *fmt++) != '\0') {
        if (ch == '%') {
            int left = 0;
            int addplus = 0;
            int alternate = 0;
            int padzeros = 0;
            size_t width = 0;
            size_t limit = -1;
            int longlong = 0;
            int half = 0;
            size_t len, i;
            int uppercase;
            void *ptr;
            const wchar_t *start = fmt;
            int prec;
            union {
                double d;
                uint32_t i[2];
            } fpval;
            int dsgn, expt, ndig;
            char *cp, *rve, *bp;
 
            if ((ch = *fmt++) == '\0')
                goto out;
 
            for (;;) {
                if (ch == '-')
                    left = 1;
                else if (ch == '+')
                    addplus = 1;
                else if (ch == '#')
                    alternate = 1;
                else if (ch == '0')
                    padzeros = 1;
                else
                    break;
 
                if ((ch = *fmt++) == '\0')
                    goto out;
            }
 
            while (ch >= '0' && ch <= '9') {
                width = 10 * width + (ch - '0');
 
                if ((ch = *fmt++) == '\0')
                    goto out;
            }
 
            if (ch == '.') {
                if ((ch = *fmt++) == '\0')
                    goto out;
 
                if (ch == '*') {
                    limit = va_arg(ap, size_t);
 
                    if ((ch = *fmt++) == '\0')
                        goto out;
                } else {
                    limit = 0;
                    while (ch >= '0' && ch <= '9') {
                        limit = 10 * limit + (ch - '0');
 
                        if ((ch = *fmt++) == '\0')
                            goto out;
                    }
                }
            }
 
            if (ch == 'L') {
                longlong = 1;
 
                if ((ch = *fmt++) == '\0')
                    goto out;
            } else if (ch == 'l') {
                if ((ch = *fmt++) == '\0')
                    goto out;
 
                if (ch == 'l') {
                    longlong = 1;
 
                    if ((ch = *fmt++) == '\0')
                        goto out;
                }
            } else if (ch == 'h') {
                half = 1;
 
                if ((ch = *fmt++) == '\0')
                    goto out;
            }
 
            switch (ch) {
                default:
                    fmt = start;
                    /* Fall through */
                case '%':
                    PUTCH('%');
                    break;
                case 'C':
                case 'c':
                    PUTCH(va_arg(ap, wchar_t));
                    break;
                case 'D':
                case 'd':
                    uppercase = (ch == 'D');
                    if (longlong)
                        len = lltoa(va_arg(ap, long long), tmp, 10, 1, addplus, uppercase);
                    else
                        len = itoa(va_arg(ap, int), tmp, 10, 1, addplus, uppercase);
 
                    hsrc = tmp;
 
                    if (width > len)
                        width -= len;
                    else
                        width = 0;
 
                    if (left == 0) {
                        for (i = 0; i < width; i++) PUTCH(padzeros ? '0' : ' ');
                    }
 
                    for (i = 0; i < len; i++) PUTCH(hsrc[i]);
 
                    if (left != 0) {
                        for (i = 0; i < width; i++) PUTCH(' ');
                    }
                    break;
                case 'U':
                case 'u':
                    uppercase = (ch == 'U');
                    if (longlong)
                        len = lltoa(va_arg(ap, unsigned long long), tmp, 10, 0, addplus, uppercase);
                    else
                        len = itoa(va_arg(ap, unsigned int), tmp, 10, 0, addplus, uppercase);
 
                    hsrc = tmp;
 
                    if (width > len)
                        width -= len;
                    else
                        width = 0;
 
                    if (left == 0) {
                        for (i = 0; i < width; i++) PUTCH(padzeros ? '0' : ' ');
                    }
 
                    for (i = 0; i < len; i++) PUTCH(hsrc[i]);
 
                    if (left != 0) {
                        for (i = 0; i < width; i++) PUTCH(' ');
                    }
                    break;
                case 'X':
                case 'x':
                    uppercase = (ch == 'X');
                    if (longlong)
                        len = lltoa(va_arg(ap, unsigned long long), tmp, 16, 0, addplus, uppercase);
                    else
                        len = itoa(va_arg(ap, unsigned int), tmp, 16, 0, addplus, uppercase);
 
                    hsrc = tmp;
 
                    if (width > len)
                        width -= len;
                    else
                        width = 0;
 
                    if (left == 0) {
                        for (i = 0; i < width; i++) PUTCH(padzeros ? '0' : ' ');
                    }
 
                    for (i = 0; i < len; i++) PUTCH(hsrc[i]);
 
                    if (left != 0) {
                        for (i = 0; i < width; i++) PUTCH(' ');
                    }
                    break;
                case 'P':
                case 'p':
                    uppercase = (ch == 'P');
                    ptr = va_arg(ap, void *);
                    len = itoa((unsigned)ptr, tmp, 16, 0, 0, uppercase);
 
                    hsrc = tmp;
                    width = 8;
 
                    if (width > len)
                        width -= len;
                    else
                        width = 0;
 
                    if (alternate && ptr != NULL) {
                        PUTCH('0');
                        PUTCH('x');
                    }
 
                    for (i = 0; i < width; i++) PUTCH('0');
 
                    for (i = 0; i < len; i++) PUTCH(hsrc[i]);
                    break;
                case 'S':
                case 's':
                    if (half) {
                        hsrc = va_arg(ap, const char *);
                        if (hsrc == NULL)
                            hsrc = "(null)";
 
                        len = strlen(hsrc);
 
                        if (limit > 0 && len > limit)
                            len = limit;
 
                        if (width > len)
                            width -= len;
                        else
                            width = 0;
 
                        if (left == 0) {
                            for (i = 0; i < width; i++) PUTCH(' ');
                        }
 
                        for (i = 0; i < len; i++) PUTCH(hsrc[i]);
 
                        if (left != 0) {
                            for (i = 0; i < width; i++) PUTCH(' ');
                        }
                    } else {
                        src = va_arg(ap, const wchar_t *);
                        if (src == NULL)
                            src = L"(null)";
 
                        len = wcslen(src);
 
                        if (limit > 0 && len > limit)
                            len = limit;
 
                        if (width > len)
                            width -= len;
                        else
                            width = 0;
 
                        if (left == 0) {
                            for (i = 0; i < width; i++) PUTCH(' ');
                        }
 
                        for (i = 0; i < len; i++) PUTCH(src[i]);
 
                        if (left != 0) {
                            for (i = 0; i < width; i++) PUTCH(' ');
                        }
                    }
                    break;
                case 'F':
                case 'f':
                    prec = limit;
 
                    if (prec == -1) {
                        prec = 6;
                    }
 
                    fpval.d = va_arg(ap, double);
 
                    if (isinf(fpval.d)) {
                        if (fpval.d < 0) PUTCH('-');
 
                        if (ch == 'F') {
                            PUTCH('I');
                            PUTCH('N');
                            PUTCH('F');
                        } else {
                            PUTCH('i');
                            PUTCH('n');
                            PUTCH('f');
                        }
 
                        break;
                    }
 
                    if (isnan(fpval.d)) {
                        if (ch == 'F') {
                            PUTCH('N');
                            PUTCH('A');
                            PUTCH('N');
                        } else {
                            PUTCH('n');
                            PUTCH('a');
                            PUTCH('n');
                        }
 
                        break;
                    }
 
                    if (fpval.i[0] & 0x80000000UL) {
                        fpval.d = -fpval.d;
                        PUTCH('-');
                    }
 
                    cp = _dtoa_r(r, fpval.d, 3, prec, &expt, &dsgn, &rve);
 
                    bp = cp + prec;
 
                    if (*cp == '0' && fpval.d)
                        expt = -prec + 1;
                    bp += expt;
 
                    if (fpval.d == 0)
                        rve = bp;
 
                    while (rve < bp)
                        *rve++ = '0';
 
                    ndig = rve - cp;
 
                    if (fpval.d == 0) {
                        PUTCH('0');
                        if (expt < ndig || alternate) {
                            PUTCH('.');
                            for (i = 0; i < (ndig - 1); i++) PUTCH('0');
                        }
                    } else if (expt <= 0) {
                        PUTCH('0');
                        if (expt || ndig) {
                            PUTCH('.');
                            for (i = 0; i < -expt; i++) PUTCH('0');
                            for (i = 0; i < ndig; i++) PUTCH(cp[i]);
                        }
                    } else if (expt >= ndig) {
                        for (i = 0; i < ndig; i++) PUTCH(cp[i]);
                        for (i = 0; i < (expt - ndig); i++) PUTCH('0');
                        if (alternate) PUTCH('.');
                    } else {
                        for (i = 0; i < expt; i++) PUTCH(cp[i]);
                        PUTCH('.');
                        for (i = expt; i < ndig; i++) PUTCH(cp[i]);
                    }
 
                    break;
            }
        } else {
            PUTCH(ch);
        }
    }
 
out:
 
    PUTCH('\0');
 
    return pcd.count;
}
 
int swprintf(wchar_t *buffer, size_t maxlen, const wchar_t *fmt, ...) {
    va_list ap;
    int wc;
 
    va_start(ap, fmt);
    wc = vswprintf(buffer, maxlen, fmt, ap);
    va_end(ap);
 
    return wc;
}
