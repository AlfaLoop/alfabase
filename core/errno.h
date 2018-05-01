/**
 * © Copyright AlfaLoop Technology Co., Ltd. 2018
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **/
#ifndef _ERRNO_H
#define _ERRNO_H

#ifdef __cplusplus
extern "C" {
#endif

// Global error codes are listed below.
//
#define ENONE		 	0   /* No error occur */

// System
#define EINTERNAL    	1   /* Internal Error */
#define ENOMEM       	2   /* No Memory for operation */
#define ENULLP       	3   /* Null Pointer */
#define EINVAL       	4   /* Invalid argument */
#define ENOREADY	 	  5   /* Not ready */
#define EINVALSTATE  	6   /* Invalid state, operation disallowed in this state */
#define ENOFOUND     	7   /* Not found */
#define ENOSUPPORT   	8   /* Not supported */
#define ETIME        	9   /* Timer expired (timeout) */
#define EPERM        	10  /* Operation not permitted */
#define EIO      	  	11  /* I/O error */
#define EFAULT       	12  /* Bad address */
#define EBUSY        	13  /* Device or resource busy */
#define EBADRQC      	14  /* Invalid request code */
#define EALREADY     	15  /* Operation already in progress */
#define EOVERFLOW    	16  /* Value too large for defined data type */
#define ELOAR			    17	/* Limit of available resource */
#define EDSNM			    18	/* Data size not match */
#define EAGAIN        19     /* Try again */
#define EWOULDBLOCK EAGAIN  /* Operation would block */


// File System
#define ENOENT       	30  /* No such file or directory */
#define EBADF        	31  /* Bad file number */
#define EROFS        	32  /* Read-only file system */
#define ENOTDIR      	33  /* Not a directory */
#define E2BIG        	34  /* Argument list too long */
#define ENOEXEC      	35  /* Exec format error */
#define EEXIST       	36  /* File exists */
#define EISDIR       	37  /* Is a directory */
#define EAPPEN       	38  /* Append faild */
#define EISCA         39  /* Insufficient storage capacity available.*/

// ELF Loader
#define EBEH       		40  /* Bad ELF header */
#define ENSYMT       	41  /* No symbol table */
#define ENSTRT       	42  /* No string table */
#define ENTXTSG       43  /* No text segment */
#define ESYMNF       	44  /* Symbol not found */
#define ESGNF       	45  /* Segment not found */
#define ENSTARTP      46  /* No starting point */

// Network
#define ENONET      	50  /* Machine is not on the network */
#define EADV        	51  /* Advertise error */
#define ECOMM       	52  /* Communication error on send */
#define EPROTO      	53  /* Protocol error */
#define EMULTIHOP   	54  /* Multihop attempted */
#define EBADMSG     	55  /* Not a data message */
#define EMSGSIZE    	56  /* Message too long */
#define ENOPROTOOPT 	57  /* Protocol not available */
#define EPROTONOSUPPORT 58  /* Protocol not supported */
#define ENETDOWN    	59  /* Network is down */
#define ENETUNREACH 	60  /* Network is unreachable */
#define ENETRESET   	61  /* Network dropped connection because of reset */
#define ECONNRESET  	62  /* Connection reset by peer */
#define ETIMEDOUT   	63  /* Connection timed out */
#define ECONNREFUSED  64  /* Connection refused */
#define EHOSTDOWN   	65  /* Host is down */
#define EHOSTUNREACH  66  /* No route to host */


#ifdef __cplusplus
}
#endif

#endif /* _ERRNO_H */
