/*
 *  DNS Resource record manipulation routines
 *
 *      Translation routines - internal use only
 #              (i.e. undocumented!)
 *
 *  Dave Shield         November 1993
 */

#ifndef INCLUDE_RES_INFO_H
#define INCLUDE_RES_INFO_H
#ifdef __cplusplus
extern "C" {
#endif

extern const char * res_opcode(int i);
extern const char * res_rcode(int i);
extern const char * res_wks(int i);
extern const char * res_proto(int i);
extern const char * res_type(int i);
extern int    which_res_type(const char* s);
extern const char * res_class(int i);
void res_time(char* retbuf, int bufferSize, int value);

#ifdef __cplusplus
}
#endif
#endif /* INCLUDE_RES_INFO_H */
