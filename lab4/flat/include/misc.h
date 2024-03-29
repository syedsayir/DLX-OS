//
//	Routines used by the entire operating system.
//

#ifndef	_misc_h_
#define	_misc_h_

extern char*	dstrcpy(char*, const char*);
extern char*	dstrncpy(char*, const char*, int n);
extern const char *dstrstr (const char *, const char *);
extern int	dstrlen (const char *);
extern int	dstrncmp (const char *, const char *, int);
extern int	dstrtol(char*, char**, int);
extern const char*	dindex(const char*, int);
extern const char*	dmindex(const char*, const char *);
extern void ditoa (int n, char *buf);
char *dstrcat (char *onto, const char *addn);
int min(int a, int b);
int max(int a, int b);
void bzero(char *mem, int num_bytes);
void bcopy(char *src, char *dst, int num_bytes);

inline
int
isspace (char c)
{
  return ((c == ' ') || (c == '\t') || (c == '\n'));
}

inline
int
isxdigit (char c)
{
  return (((c >= '0') && (c <= '9')) ||
	  ((c >= 'a') && (c <= 'f')) ||
	  ((c >= 'A') && (c <= 'A')));
}

int ceil(float num) {
    int inum = (int)num;
    if (num == (float)inum) {
        return inum;
    }
    return inum + 1;
}

int StrCmp(char *a,char *b)
{	int i=0;
	while(a[i]==b[i])
	{
		if(a[i]=='\0')
		return 1;
		i++;

	}
	return 0;
}

#endif	// !_misc_h_
