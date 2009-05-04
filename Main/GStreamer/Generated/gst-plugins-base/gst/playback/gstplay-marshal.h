
#ifndef __gst_play_marshal_MARSHAL_H__
#define __gst_play_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* BOOLEAN:OBJECT,BOXED */
extern void gst_play_marshal_BOOLEAN__OBJECT_BOXED (GClosure     *closure,
                                                    GValue       *return_value,
                                                    guint         n_param_values,
                                                    const GValue *param_values,
                                                    gpointer      invocation_hint,
                                                    gpointer      marshal_data);

/* BOOLEAN:OBJECT,OBJECT,OBJECT */
extern void gst_play_marshal_BOOLEAN__OBJECT_OBJECT_OBJECT (GClosure     *closure,
                                                            GValue       *return_value,
                                                            guint         n_param_values,
                                                            const GValue *param_values,
                                                            gpointer      invocation_hint,
                                                            gpointer      marshal_data);

/* BOXED:OBJECT,BOXED */
extern void gst_play_marshal_BOXED__OBJECT_BOXED (GClosure     *closure,
                                                  GValue       *return_value,
                                                  guint         n_param_values,
                                                  const GValue *param_values,
                                                  gpointer      invocation_hint,
                                                  gpointer      marshal_data);

/* VOID:OBJECT,BOOLEAN */
extern void gst_play_marshal_VOID__OBJECT_BOOLEAN (GClosure     *closure,
                                                   GValue       *return_value,
                                                   guint         n_param_values,
                                                   const GValue *param_values,
                                                   gpointer      invocation_hint,
                                                   gpointer      marshal_data);

/* ENUM:OBJECT,OBJECT,BOXED */
extern void gst_play_marshal_ENUM__OBJECT_OBJECT_BOXED (GClosure     *closure,
                                                        GValue       *return_value,
                                                        guint         n_param_values,
                                                        const GValue *param_values,
                                                        gpointer      invocation_hint,
                                                        gpointer      marshal_data);

/* ENUM:OBJECT,BOXED,OBJECT */
extern void gst_play_marshal_ENUM__OBJECT_BOXED_OBJECT (GClosure     *closure,
                                                        GValue       *return_value,
                                                        guint         n_param_values,
                                                        const GValue *param_values,
                                                        gpointer      invocation_hint,
                                                        gpointer      marshal_data);

/* BOXED:OBJECT,BOXED,BOXED */
extern void gst_play_marshal_BOXED__OBJECT_BOXED_BOXED (GClosure     *closure,
                                                        GValue       *return_value,
                                                        guint         n_param_values,
                                                        const GValue *param_values,
                                                        gpointer      invocation_hint,
                                                        gpointer      marshal_data);

/* BOXED:INT */
extern void gst_play_marshal_BOXED__INT (GClosure     *closure,
                                         GValue       *return_value,
                                         guint         n_param_values,
                                         const GValue *param_values,
                                         gpointer      invocation_hint,
                                         gpointer      marshal_data);

/* OBJECT:BOXED */
extern void gst_play_marshal_OBJECT__BOXED (GClosure     *closure,
                                            GValue       *return_value,
                                            guint         n_param_values,
                                            const GValue *param_values,
                                            gpointer      invocation_hint,
                                            gpointer      marshal_data);

/* OBJECT:INT */
extern void gst_play_marshal_OBJECT__INT (GClosure     *closure,
                                          GValue       *return_value,
                                          guint         n_param_values,
                                          const GValue *param_values,
                                          gpointer      invocation_hint,
                                          gpointer      marshal_data);

G_END_DECLS

#endif /* __gst_play_marshal_MARSHAL_H__ */

