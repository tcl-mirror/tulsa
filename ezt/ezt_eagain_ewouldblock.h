/*** ezt_eagain_ewouldblock.h:begin ***/
/*
 * Make sure that both EAGAIN and EWOULDBLOCK are defined.
 * This does not compile on systems where neither is defined.
 * We want both defined so that we can test safely for both.
 * In the code we still have to test for both because there may
 * be systems on which both are defined and have different values.
 */
#if ((!defined(EWOULDBLOCK)) && (defined(EAGAIN)))
#   define EWOULDBLOCK EAGAIN
#endif
#if ((!defined(EAGAIN)) && (defined(EWOULDBLOCK)))
#   define EAGAIN EWOULDBLOCK
#endif
#if ((!defined(EAGAIN)) && (!defined(EWOULDBLOCK)))
#error one of EWOULDBLOCK or EAGAIN must be defined
#endif
/*** ezt_eagain_ewouldblock.h:begin ***/
