
#ifndef __gnl_marshal_MARSHAL_H__
#define __gnl_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* VOID:OBJECT,UINT (gnlmarshal.list:1) */
extern void gnl_marshal_VOID__OBJECT_UINT (GClosure     *closure,
                                           GValue       *return_value,
                                           guint         n_param_values,
                                           const GValue *param_values,
                                           gpointer      invocation_hint,
                                           gpointer      marshal_data);

G_END_DECLS

#endif /* __gnl_marshal_MARSHAL_H__ */

