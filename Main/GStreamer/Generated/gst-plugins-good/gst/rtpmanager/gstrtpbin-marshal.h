
#ifndef __gst_rtp_bin_marshal_MARSHAL_H__
#define __gst_rtp_bin_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* UINT:UINT */
extern void gst_rtp_bin_marshal_UINT__UINT (GClosure     *closure,
                                            GValue       *return_value,
                                            guint         n_param_values,
                                            const GValue *param_values,
                                            gpointer      invocation_hint,
                                            gpointer      marshal_data);

/* BOXED:UINT */
extern void gst_rtp_bin_marshal_BOXED__UINT (GClosure     *closure,
                                             GValue       *return_value,
                                             guint         n_param_values,
                                             const GValue *param_values,
                                             gpointer      invocation_hint,
                                             gpointer      marshal_data);

/* BOXED:UINT,UINT */
extern void gst_rtp_bin_marshal_BOXED__UINT_UINT (GClosure     *closure,
                                                  GValue       *return_value,
                                                  guint         n_param_values,
                                                  const GValue *param_values,
                                                  gpointer      invocation_hint,
                                                  gpointer      marshal_data);

/* OBJECT:UINT */
extern void gst_rtp_bin_marshal_OBJECT__UINT (GClosure     *closure,
                                              GValue       *return_value,
                                              guint         n_param_values,
                                              const GValue *param_values,
                                              gpointer      invocation_hint,
                                              gpointer      marshal_data);

/* VOID:UINT,OBJECT */
extern void gst_rtp_bin_marshal_VOID__UINT_OBJECT (GClosure     *closure,
                                                   GValue       *return_value,
                                                   guint         n_param_values,
                                                   const GValue *param_values,
                                                   gpointer      invocation_hint,
                                                   gpointer      marshal_data);

/* VOID:UINT */
#define gst_rtp_bin_marshal_VOID__UINT	g_cclosure_marshal_VOID__UINT

/* VOID:UINT,UINT */
extern void gst_rtp_bin_marshal_VOID__UINT_UINT (GClosure     *closure,
                                                 GValue       *return_value,
                                                 guint         n_param_values,
                                                 const GValue *param_values,
                                                 gpointer      invocation_hint,
                                                 gpointer      marshal_data);

/* VOID:OBJECT,OBJECT */
extern void gst_rtp_bin_marshal_VOID__OBJECT_OBJECT (GClosure     *closure,
                                                     GValue       *return_value,
                                                     guint         n_param_values,
                                                     const GValue *param_values,
                                                     gpointer      invocation_hint,
                                                     gpointer      marshal_data);

G_END_DECLS

#endif /* __gst_rtp_bin_marshal_MARSHAL_H__ */

