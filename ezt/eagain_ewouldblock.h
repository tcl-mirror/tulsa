/*** eagain_ewouldblock.h:begin ***/
#if ((!defined(EWOULDBLOCK)) && (defined(EAGAIN)))
#   define EWOULDBLOCK EAGAIN
#endif
#if ((!defined(EAGAIN)) && (defined(EWOULDBLOCK)))
#   define EAGAIN EWOULDBLOCK
#endif
#if ((!defined(EAGAIN)) && (!defined(EWOULDBLOCK)))
#error one of EWOULDBLOCK or EAGAIN must be defined
#endif
/*** eagain_ewouldblock.h:begin ***/
