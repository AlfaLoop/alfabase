#ifndef __PREFIX_H_
#define __PREFIX_H_


#ifdef __cplusplus
extern "C" {
#endif


#define CCIF
#define CLIF

typedef unsigned short lc_t;
#define LC_INIT(s) s = 0;
#define LC_RESUME(s) switch(s) { case 0:
#define LC_SET(s) s = __LINE__; case __LINE__:
#define LC_END(s) }

struct pt {
  lc_t lc;
};

#define PT_WAITING 0
#define PT_YIELDED 1
#define PT_EXITED  2
#define PT_ENDED   3

#define PT_INIT(pt)   LC_INIT((pt)->lc)
#define PT_THREAD(name_args) char name_args
#define PT_BEGIN(pt) { char PT_YIELD_FLAG = 1; if (PT_YIELD_FLAG) {;} LC_RESUME((pt)->lc)
#define PT_END(pt) LC_END((pt)->lc); PT_YIELD_FLAG = 0; \
                   PT_INIT(pt); return PT_ENDED; }

#define PT_WAIT_UNTIL(pt, condition)          \
  do {            \
    LC_SET((pt)->lc);       \
    if(!(condition)) {        \
      return PT_WAITING;      \
    }           \
  } while(0)

#define PT_WAIT_WHILE(pt, cond)  PT_WAIT_UNTIL((pt), !(cond))

#define PT_WAIT_THREAD(pt, thread) PT_WAIT_WHILE((pt), PT_SCHEDULE(thread))

#define PT_SPAWN(pt, child, thread)   \
  do {            \
    PT_INIT((child));       \
    PT_WAIT_THREAD((pt), (thread));   \
  } while(0)

#define PT_RESTART(pt)        \
  do {            \
    PT_INIT(pt);        \
    return PT_WAITING;      \
  } while(0)

#define PT_EXIT(pt)       \
  do {            \
    PT_INIT(pt);        \
    return PT_EXITED;     \
  } while(0)

#define PT_SCHEDULE(f) ((f) < PT_EXITED)


#define PT_YIELD(pt)        \
  do {            \
    PT_YIELD_FLAG = 0;        \
    LC_SET((pt)->lc);       \
    if(PT_YIELD_FLAG == 0) {      \
      return PT_YIELDED;      \
    }           \
  } while(0)

#define PT_YIELD_UNTIL(pt, cond)    \
  do {            \
    PT_YIELD_FLAG = 0;        \
    LC_SET((pt)->lc);       \
    if((PT_YIELD_FLAG == 0) || !(cond)) { \
      return PT_YIELDED;      \
    }           \
  } while(0)


#define CC_CONF_REGISTER_ARGS          0
#define CC_CONF_FUNCTION_POINTER_ARGS  1
#define CC_CONF_FASTCALL
#define CC_CONF_VA_ARGS                1
#define CC_CONF_INLINE                 inline


#if CC_CONF_REGISTER_ARGS
#define CC_REGISTER_ARG register
#else /* CC_CONF_REGISTER_ARGS */
#define CC_REGISTER_ARG
#endif /* CC_CONF_REGISTER_ARGS */


#if CC_CONF_FUNCTION_POINTER_ARGS
#define CC_FUNCTION_POINTER_ARGS 1
#else /* CC_CONF_FUNCTION_POINTER_ARGS */
#define CC_FUNCTION_POINTER_ARGS 0
#endif /* CC_CONF_FUNCTION_POINTER_ARGS */

#ifdef CC_CONF_FASTCALL
#define CC_FASTCALL CC_CONF_FASTCALL
#else /* CC_CONF_FASTCALL */
#define CC_FASTCALL
#endif /* CC_CONF_FASTCALL */

#ifdef CC_CONF_CONST_FUNCTION_BUG
#define CC_CONST_FUNCTION
#else /* CC_CONF_FASTCALL */
#define CC_CONST_FUNCTION const
#endif /* CC_CONF_FASTCALL */


#if CC_CONF_UNSIGNED_CHAR_BUGS
#define CC_UNSIGNED_CHAR_BUGS 1
#else /* CC_CONF_UNSIGNED_CHAR_BUGS */
#define CC_UNSIGNED_CHAR_BUGS 0
#endif /* CC_CONF_UNSIGNED_CHAR_BUGS */

#if CC_CONF_DOUBLE_HASH
#define CC_DOUBLE_HASH 1
#else /* CC_CONF_DOUBLE_HASH */
#define CC_DOUBLE_HASH 0
#endif /* CC_CONF_DOUBLE_HASH */

#ifdef CC_CONF_INLINE
#define CC_INLINE CC_CONF_INLINE
#else /* CC_CONF_INLINE */
#define CC_INLINE
#endif /* CC_CONF_INLINE */


#ifdef CC_CONF_ASSIGN_AGGREGATE
#define CC_ASSIGN_AGGREGATE(dest, src)  CC_CONF_ASSIGN_AGGREGATE(dest, src)
#else /* CC_CONF_ASSIGN_AGGREGATE */
#define CC_ASSIGN_AGGREGATE(dest, src)  *dest = *src
#endif /* CC_CONF_ASSIGN_AGGREGATE */

#if CC_CONF_NO_VA_ARGS
#define CC_NO_VA_ARGS CC_CONF_VA_ARGS
#endif

#ifndef NULL
#define NULL 0
#endif /* NULL */

#ifndef MAX
#define MAX(n, m)   (((n) < (m)) ? (m) : (n))
#endif

#ifndef MIN
#define MIN(n, m)   (((n) < (m)) ? (n) : (m))
#endif

#ifndef ABS
#define ABS(n)      (((n) < 0) ? -(n) : (n))
#endif

#define CC_CONCAT2(s1, s2) s1##s2
#define CC_CONCAT(s1, s2) CC_CONCAT2(s1, s2)


typedef unsigned char process_event_t;
typedef void *        process_data_t;
typedef unsigned char process_num_events_t;

#define PROCESS_ERR_OK        0

#define PROCESS_ERR_FULL      1


#define PROCESS_NONE          NULL

#ifndef PROCESS_CONF_NUMEVENTS
#define PROCESS_CONF_NUMEVENTS 32
#endif /* PROCESS_CONF_NUMEVENTS */

#define PROCESS_EVENT_NONE            0x80
#define PROCESS_EVENT_INIT            0x81
#define PROCESS_EVENT_POLL            0x82
#define PROCESS_EVENT_EXIT            0x83
#define PROCESS_EVENT_CONTINUE        0x84
#define PROCESS_EVENT_MSG             0x85
#define PROCESS_EVENT_EXITED          0x86
#define PROCESS_EVENT_TIMER           0x87
#define PROCESS_EVENT_COM             0x88
#define PROCESS_EVENT_MAX             0x89

#define PROCESS_BROADCAST NULL
#define PROCESS_ZOMBIE ((struct process *)0x1)

#define PROCESS_BEGIN()             PT_BEGIN(process_pt)

#define PROCESS_END()               PT_END(process_pt)

#define PROCESS_WAIT_EVENT()        PROCESS_YIELD()

#define PROCESS_WAIT_EVENT_UNTIL(c) PROCESS_YIELD_UNTIL(c)

#define PROCESS_YIELD()             PT_YIELD(process_pt)

#define PROCESS_YIELD_UNTIL(c)      PT_YIELD_UNTIL(process_pt, c)

#define PROCESS_WAIT_UNTIL(c)       PT_WAIT_UNTIL(process_pt, c)
#define PROCESS_WAIT_WHILE(c)       PT_WAIT_WHILE(process_pt, c)

#define PROCESS_EXIT()              PT_EXIT(process_pt)

#define PROCESS_PT_SPAWN(pt, thread)   PT_SPAWN(process_pt, pt, thread)

#define PROCESS_PAUSE()             do {        \
  process_post(PROCESS_CURRENT(), PROCESS_EVENT_CONTINUE, NULL);  \
  PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_CONTINUE);               \
} while(0)

#define PROCESS_POLLHANDLER(handler) if(ev == PROCESS_EVENT_POLL) { handler; }

#define PROCESS_EXITHANDLER(handler) if(ev == PROCESS_EVENT_EXIT) { handler; }

#define PROCESS_THREAD(name, ev, data)        \
static PT_THREAD(process_thread_##name(struct pt *process_pt, \
               process_event_t ev,  \
               process_data_t data))

#define PROCESS_NAME(name) extern struct process name

#if PROCESS_CONF_NO_PROCESS_NAMES
#define PROCESS(name, strname)        \
  PROCESS_THREAD(name, ev, data);     \
  struct process name = { NULL,           \
                          process_thread_##name }
#else
#define PROCESS(name, strname)        \
  PROCESS_THREAD(name, ev, data);     \
  struct process name = { NULL, strname,    \
                          process_thread_##name }
#endif

struct process {
  struct process *next;
#if PROCESS_CONF_NO_PROCESS_NAMES
#define PROCESS_NAME_STRING(process) ""
#else
  const char *name;
#define PROCESS_NAME_STRING(process) (process)->name
#endif
  PT_THREAD((* thread)(struct pt *, process_event_t, process_data_t));
  struct pt pt;
  unsigned char state, needspoll;
};


#define CC_CONF_REGISTER_ARGS          0
#define CC_CONF_FUNCTION_POINTER_ARGS  1
#define CC_CONF_FASTCALL
#define CC_CONF_VA_ARGS                1
#define CC_CONF_INLINE                 inline

#define AUTOSTART_ENABLE 1
#if ! CC_NO_VA_ARGS
#if AUTOSTART_ENABLE
#define AUTOSTART_PROCESSES(...)          \
struct process * const autostart_processes[] = {__VA_ARGS__, NULL}
#else /* AUTOSTART_ENABLE */
#define AUTOSTART_PROCESSES(...)          \
extern int _dummy
#endif /* AUTOSTART_ENABLE */
#else
#error "C compiler must support __VA_ARGS__ macro"
#endif

CLIF extern struct process * const autostart_processes[];

typedef int (*OSProcessEntry_t)(void);
int OSProcessEntrySetup(OSProcessEntry_t p_entry);

#ifdef __cplusplus
}
#endif

#endif /* __PREFIX_H_ */
