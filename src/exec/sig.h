#ifndef PRIVATE__SIG_H__INCLUDED__
#define PRIVATE__SIG_H__INCLUDED__

extern int sigcapture__(void (*handler)(int, void *), void *cookie);
extern int sigrestore__(void);

#endif /* PRIVATE__SIG_H__INCLUDED__ */
