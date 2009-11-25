// Wrapper for Gst.GLib.Value to add support for GstFraction, GstFourcc, Gst*Range, ...

using System;
using System.Text;
using System.Collections;
using System.Runtime.InteropServices;
using Gst.GLib;

/* TODO: intersect, compare, substract, .... */
namespace Gst {
  public struct Fraction {
    public int Numerator {
      get {
        return numerator;
      }

      set {
        numerator = value;
        Reduce ();
      }
    }

    public int Denominator {
      get {
        return denominator;
      }

      set {
        if (denominator == 0)
          throw new ArgumentException ();

        denominator = value;
        Reduce ();
      }
    }

    private int numerator;
    private int denominator;

    public static Gst.GLib.GType GType {
      get {
        return new GType (gst_fraction_get_type ());
      }
    }

    private void Reduce () {
      int gcd = GreatestCommonDivisor (this);

      if (gcd != 0) {
        this.numerator /= gcd;
        this.denominator /= gcd;
      }
    }

    private static int GreatestCommonDivisor (Fraction fraction) {
      int a = fraction.numerator;
      int b = fraction.denominator;

      while (b != 0) {
        int temp = a;

        a = b;
        b = temp % b;
      }
      return Math.Abs (a);
    }

    public Fraction (int numerator, int denominator) {
      if (denominator == 0)
        throw new ArgumentException ();

      this.numerator = numerator;
      this.denominator = denominator;
      Reduce ();
    }

    public Fraction (Gst.GLib.Value val) : this () {
      this.numerator = gst_value_get_fraction_numerator (ref val);
      this.denominator = gst_value_get_fraction_denominator (ref val);
    }

    public void SetGValue (ref Gst.GLib.Value val) {
      gst_value_set_fraction (ref val, Numerator, Denominator);
    }

    public override string ToString () {
      return String.Format ("{0}/{1}", numerator, denominator);
    }

    public static explicit operator Gst.GLib.Value (Fraction fraction) {
      Gst.GLib.Value val = new Gst.GLib.Value (Fraction.GType);
      gst_value_set_fraction (ref val, fraction.Numerator, fraction.Denominator);

      return val;
    }

    public static explicit operator double (Fraction fraction) {
      return ( (double) fraction.numerator) / ( (double) fraction.denominator);
    }

    public static Fraction operator + (Fraction a, Fraction b) {
      return new Fraction ( (a.Numerator * b.Denominator) + (b.Numerator * a.Denominator), a.Denominator * b.Denominator);
    }

    public static Fraction operator - (Fraction a, Fraction b) {
      return new Fraction ( (a.Numerator * b.Denominator) - (b.Numerator * a.Denominator), a.Denominator * b.Denominator);
    }

    public static Fraction operator * (Fraction a, Fraction b) {
      return new Fraction (a.Numerator * b.Numerator, a.Denominator * b.Denominator);
    }

    public static Fraction operator / (Fraction a, Fraction b) {
      return new Fraction (a.Numerator * b.Denominator, a.Denominator * b.Numerator);
    }

    [DllImport ("libgstreamer-0.10.dll") ]
    private static extern void gst_value_set_fraction (ref Gst.GLib.Value v, int numerator, int denominator);
    [DllImport ("libgstreamer-0.10.dll") ]
    private static extern int gst_value_get_fraction_numerator (ref Gst.GLib.Value v);
    [DllImport ("libgstreamer-0.10.dll") ]
    private static extern int gst_value_get_fraction_denominator (ref Gst.GLib.Value v);
    [DllImport ("libgstreamer-0.10.dll") ]
    private static extern IntPtr gst_fraction_get_type ();
  }

  public struct DoubleRange {
    public double Min;
    public double Max;

    public static Gst.GLib.GType GType {
      get {
        return new GType (gst_double_range_get_type ());
      }
    }

    public DoubleRange (double min, double max) {
      if (min > max)
        throw new ArgumentException ();

      this.Min = min;
      this.Max = max;
    }

    public DoubleRange (Gst.GLib.Value val) : this () {
      this.Min = gst_value_get_double_range_min (ref val);
      this.Max = gst_value_get_double_range_max (ref val);
    }

    public override string ToString () {
      return String.Format ("[{0}, {1}]", Min, Max);
    }

    public void SetGValue (ref Gst.GLib.Value val) {
      gst_value_set_double_range (ref val, Min, Max);
    }

    public static explicit operator Gst.GLib.Value (DoubleRange range) {
      Gst.GLib.Value val = new Gst.GLib.Value (DoubleRange.GType);

      gst_value_set_double_range (ref val, range.Min, range.Max);
      return val;
    }

    [DllImport ("libgstreamer-0.10.dll") ]
    private static extern IntPtr gst_double_range_get_type ();
    [DllImport ("libgstreamer-0.10.dll") ]
    private static extern void gst_value_set_double_range (ref Gst.GLib.Value v, double min, double max);
    [DllImport ("libgstreamer-0.10.dll") ]
    private static extern double gst_value_get_double_range_min (ref Gst.GLib.Value v);
    [DllImport ("libgstreamer-0.10.dll") ]
    private static extern double gst_value_get_double_range_max (ref Gst.GLib.Value v);
  }

  public struct IntRange {
    public int Min;
    public int Max;

    public static Gst.GLib.GType GType {
      get {
        return new GType (gst_int_range_get_type ());
      }
    }

    public IntRange (int min, int max) {
      if (min > max)
        throw new ArgumentException ();

      this.Min = min;
      this.Max = max;
    }

    public IntRange (Gst.GLib.Value val) : this () {
      this.Min = gst_value_get_int_range_min (ref val);
      this.Max = gst_value_get_int_range_max (ref val);
    }

    public void SetGValue (ref Gst.GLib.Value val) {
      gst_value_set_int_range (ref val, Min, Max);
    }

    public override string ToString () {
      return String.Format ("[{0}, {1}]", Min, Max);
    }

    public static explicit operator Gst.GLib.Value (IntRange range) {
      Gst.GLib.Value val = new Gst.GLib.Value (IntRange.GType);

      gst_value_set_int_range (ref val, range.Min, range.Max);
      return val;
    }

    [DllImport ("libgstreamer-0.10.dll") ]
    private static extern IntPtr gst_int_range_get_type ();
    [DllImport ("libgstreamer-0.10.dll") ]
    private static extern void gst_value_set_int_range (ref Gst.GLib.Value v, int min, int max);
    [DllImport ("libgstreamer-0.10.dll") ]
    private static extern int gst_value_get_int_range_min (ref Gst.GLib.Value v);
    [DllImport ("libgstreamer-0.10.dll") ]
    private static extern int gst_value_get_int_range_max (ref Gst.GLib.Value v);
  }

  public struct FractionRange {
    public Fraction Min;
    public Fraction Max;

    public static Gst.GLib.GType GType {
      get {
        return new GType (gst_fraction_range_get_type ());
      }
    }

    public FractionRange (Fraction min, Fraction max) {
      double a = (double) min, b = (double) max;

      if (a > b)
        throw new ArgumentException();

      this.Min = min;
      this.Max = max;
    }

    public FractionRange (Gst.GLib.Value val) : this () {
      IntPtr min_ptr, max_ptr;
      Gst.GLib.Value min, max;

      min_ptr = gst_value_get_fraction_range_min (ref val);
      max_ptr = gst_value_get_fraction_range_max (ref val);

      min = (Gst.GLib.Value) Marshal.PtrToStructure (min_ptr, typeof (Gst.GLib.Value));
      max = (Gst.GLib.Value) Marshal.PtrToStructure (max_ptr, typeof (Gst.GLib.Value));
      this.Min = (Fraction) min.Val;
      this.Max = (Fraction) max.Val;
    }

    public void SetGValue (ref Gst.GLib.Value val) {
      Gst.GLib.Value min = new Gst.GLib.Value (Min);
      Gst.GLib.Value max = new Gst.GLib.Value (Max);
      gst_value_set_fraction_range (ref val, ref min, ref max);
      min.Dispose ();
      max.Dispose ();
    }

    public override string ToString () {
      return String.Format ("[{0}, {1}]", Min, Max);
    }

    public static explicit operator Gst.GLib.Value (FractionRange range) {
      Gst.GLib.Value val = new Gst.GLib.Value (FractionRange.GType);

      Gst.GLib.Value min = new Gst.GLib.Value (range.Min);
      Gst.GLib.Value max = new Gst.GLib.Value (range.Max);
      gst_value_set_fraction_range (ref val, ref min, ref max);
      min.Dispose ();
      max.Dispose ();
      return val;
    }

    [DllImport ("libgstreamer-0.10.dll") ]
    private static extern IntPtr gst_fraction_range_get_type ();
    [DllImport ("libgstreamer-0.10.dll") ]
    private static extern void gst_value_set_fraction_range (ref Gst.GLib.Value v, ref Gst.GLib.Value min, ref Gst.GLib.Value max);
    [DllImport ("libgstreamer-0.10.dll") ]
    private static extern IntPtr gst_value_get_fraction_range_min (ref Gst.GLib.Value v);
    [DllImport ("libgstreamer-0.10.dll") ]
    private static extern IntPtr gst_value_get_fraction_range_max (ref Gst.GLib.Value v);
  }

  public struct Fourcc {
    public uint Val;

    public static Gst.GLib.GType GType {
      get {
        return new GType (gst_fourcc_get_type ());
      }
    }

    public Fourcc (uint fourcc) {
      this.Val = fourcc;
    }

    public Fourcc (char[] fourcc) {
      if (fourcc.Length != 4)
        throw new ArgumentException ();

      this.Val = (uint) ( ( ( (byte) fourcc[0]) << 24) |
                          ( ( (byte) fourcc[1]) << 16) |
                          ( ( (byte) fourcc[2]) << 8) |
                          ( ( (byte) fourcc[3]) << 0));
    }

    public Fourcc (string fourcc) {
      if (fourcc.Length != 4)
        throw new ArgumentException ();

      this.Val = (uint) ( ( ( (byte) fourcc[0]) << 24) |
                          ( ( (byte) fourcc[1]) << 16) |
                          ( ( (byte) fourcc[2]) << 8) |
                          ( ( (byte) fourcc[3]) << 0));
    }

    public Fourcc (Gst.GLib.Value val) : this (gst_value_get_fourcc (ref val)) {
    }

    public void SetGValue (ref Gst.GLib.Value val) {
      gst_value_set_fourcc (ref val, Val);
    }

    public override string ToString () {
      return String.Format ("{0}{1}{2}{3}", (char) ( (Val >> 24) & 0xff),
                            (char) ( (Val >> 16) & 0xff),
                            (char) ( (Val >> 8) & 0xff),
                            (char) ( (Val >> 0) & 0xff));
    }

    public static explicit operator Gst.GLib.Value (Fourcc fourcc) {
      Gst.GLib.Value val = new Gst.GLib.Value (Fourcc.GType);

      gst_value_set_fourcc (ref val, fourcc.Val);
      return val;
    }

    public static explicit operator char[] (Fourcc fourcc) {
      return new char[] { (char) ( (fourcc.Val >> 24) & 0xff),
                          (char) ( (fourcc.Val >> 16) & 0xff),
                          (char) ( (fourcc.Val >> 8) & 0xff),
                          (char) ( (fourcc.Val >> 0) & 0xff)
                        };
    }

    public static explicit operator string (Fourcc fourcc) {
      return fourcc.ToString ();
    }

    [DllImport ("libgstreamer-0.10.dll") ]
    private static extern IntPtr gst_fourcc_get_type ();
    [DllImport ("libgstreamer-0.10.dll") ]
    private static extern void gst_value_set_fourcc (ref Gst.GLib.Value v, uint fourcc);
    [DllImport ("libgstreamer-0.10.dll") ]
    private static extern uint gst_value_get_fourcc (ref Gst.GLib.Value v);

  }

  public class Date : IWrapper {
    public DateTime Val;

    private IntPtr handle;
    public IntPtr Handle {
      get {
        return handle;
      }
    }

    public static Gst.GLib.GType GType {
      get {
        return new GType (gst_date_get_type ());
      }
    }

    ~Date () {
      g_date_free (handle);
    }

    public Date (DateTime date) {
      this.Val = date;
      handle = g_date_new_dmy ( (byte) Val.Day, (int) Val.Month, (ushort) Val.Year);
    }

    public Date (int day, int month, int year) {
      this.Val = new DateTime (year, month, day);
      handle = g_date_new_dmy ( (byte) Val.Day, (int) Val.Month, (ushort) Val.Year);
    }

    public Date (Gst.GLib.Value val) {
      IntPtr date = gst_value_get_date (ref val);

      this.Val = new DateTime (g_date_get_year (date), g_date_get_month (date), g_date_get_day (date));
      handle = g_date_new_dmy ( (byte) Val.Day, (int) Val.Month, (ushort) Val.Year);
    }

    public static Date New (IntPtr date) {
      return new Date (g_date_get_day (date), g_date_get_month (date), g_date_get_year (date));
    }

    public void SetGValue (ref Gst.GLib.Value val) {
      IntPtr date_ptr = g_date_new_dmy ( (byte) Val.Day, (int) Val.Month, (ushort) Val.Year);

      gst_value_set_date (ref val, date_ptr);

      Gst.GLib.Marshaller.Free (date_ptr);
    }

    public override string ToString () {
      return String.Format ("{0}-{1}-{2}", Val.Year, Val.Month, Val.Day);
    }

    public static explicit operator Gst.GLib.Value (Date date) {
      Gst.GLib.Value val = new Gst.GLib.Value (Date.GType);

      date.SetGValue (ref val);

      return val;
    }

    [DllImport ("libglib-2.0-0.dll") ]
    private static extern byte g_date_get_day (IntPtr date);
    [DllImport ("libglib-2.0-0.dll") ]
    private static extern int g_date_get_month (IntPtr date);
    [DllImport ("libglib-2.0-0.dll") ]
    private static extern ushort g_date_get_year (IntPtr date);
    [DllImport ("libglib-2.0-0.dll") ]
    private static extern IntPtr g_date_new_dmy (byte day, int month, ushort year);
    [DllImport ("libglib-2.0-0.dll") ]
    private static extern void g_date_free (IntPtr date);

    [DllImport ("libgstreamer-0.10.dll") ]
    private static extern IntPtr gst_date_get_type ();
    [DllImport ("libgstreamer-0.10.dll") ]
    private static extern IntPtr gst_value_get_date (ref Gst.GLib.Value val);
    [DllImport ("libgstreamer-0.10.dll") ]
    private static extern void gst_value_set_date (ref Gst.GLib.Value val, IntPtr date);
  }

  public struct List : IEnumerable {
    private IList content;

    public static Gst.GLib.GType GType {
      get {
        return new GType (gst_value_list_get_type ());
      }
    }

    public List (IList content) {
      this.content = content;
    }

    public List (Gst.GLib.Value val) {
      this.content = new ArrayList ();

      uint n = gst_value_list_get_size (ref val);
      for (uint i = 0; i < n; i++) {
        IntPtr v_ptr = gst_value_list_get_value (ref val, i);
        Gst.GLib.Value v = (Gst.GLib.Value) Marshal.PtrToStructure (v_ptr, typeof (Gst.GLib.Value));
        this.content.Add (v.Val);
      }
    }

    public void SetGValue (ref Gst.GLib.Value val) {
      foreach (object o in content) {
        Gst.GLib.Value v = new Gst.GLib.Value (o);
        gst_value_list_append_value (ref val, ref v);
        v.Dispose ();
      }
    }

    public override string ToString () {
      StringBuilder sb = new StringBuilder ();

      sb.Append ("< ");
      for (int i = 0; i < content.Count; i++) {
        sb.Append (content[i]);
        if (i < content.Count - 1)
          sb.Append (", ");
      }
      sb.Append (" >");

      return sb.ToString ();
    }

    public static explicit operator Gst.GLib.Value (List l) {
      Gst.GLib.Value val = new Gst.GLib.Value (List.GType);

      foreach (object o in l.content) {
        Gst.GLib.Value v = new Gst.GLib.Value (o);
        gst_value_list_append_value (ref val, ref v);
        v.Dispose ();
      }

      return val;
    }

    public IEnumerator GetEnumerator () {
      return content.GetEnumerator ();
    }

    public object this [int index] {
      set {
        content[index] = value;
      }
      get {
        return content[index];
      }
    }

    [DllImport ("libgstreamer-0.10.dll") ]
    private static extern IntPtr gst_value_list_get_type ();
    [DllImport ("libgstreamer-0.10.dll") ]
    private static extern uint gst_value_list_get_size (ref Gst.GLib.Value val);
    [DllImport ("libgstreamer-0.10.dll") ]
    private static extern IntPtr gst_value_list_get_value (ref Gst.GLib.Value val, uint index);
    [DllImport ("libgstreamer-0.10.dll") ]
    private static extern void gst_value_list_append_value (ref Gst.GLib.Value val, ref Gst.GLib.Value el);
  }

  public struct Array : IEnumerable {
    private IList content;

    public static Gst.GLib.GType GType {
      get {
        return new GType (gst_value_array_get_type ());
      }
    }

    public Array (IList content) {
      this.content = content;
    }

    public Array (Gst.GLib.Value val) {
      this.content = new ArrayList ();

      uint n = gst_value_array_get_size (ref val);
      for (uint i = 0; i < n; i++) {
        IntPtr v_ptr = gst_value_array_get_value (ref val, i);
        Gst.GLib.Value v = (Gst.GLib.Value) Marshal.PtrToStructure (v_ptr, typeof (Gst.GLib.Value));
        this.content.Add (v.Val);
      }
    }

    public void SetGValue (ref Gst.GLib.Value val) {
      foreach (object o in content) {
        Gst.GLib.Value v = new Gst.GLib.Value (o);
        gst_value_array_append_value (ref val, ref v);
        v.Dispose ();
      }
    }

    public static explicit operator Gst.GLib.Value (Array a) {
      Gst.GLib.Value val = new Gst.GLib.Value (Gst.Array.GType);

      foreach (object o in a.content) {
        Gst.GLib.Value v = new Gst.GLib.Value (o);
        gst_value_array_append_value (ref val, ref v);
        v.Dispose ();
      }

      return val;
    }

    public override string ToString () {
      StringBuilder sb = new StringBuilder ();

      sb.Append ("{ ");
      for (int i = 0; i < content.Count; i++) {
        sb.Append (content[i]);
        if (i < content.Count - 1)
          sb.Append (", ");
      }
      sb.Append (" }");

      return sb.ToString ();
    }

    public IEnumerator GetEnumerator () {
      return content.GetEnumerator ();
    }

    public object this [int index] {
      set {
        content[index] = value;
      }
      get {
        return content[index];
      }
    }

    [DllImport ("libgstreamer-0.10.dll") ]
    private static extern IntPtr gst_value_array_get_type ();
    [DllImport ("libgstreamer-0.10.dll") ]
    private static extern uint gst_value_array_get_size (ref Gst.GLib.Value val);
    [DllImport ("libgstreamer-0.10.dll") ]
    private static extern IntPtr gst_value_array_get_value (ref Gst.GLib.Value val, uint index);
    [DllImport ("libgstreamer-0.10.dll") ]
    private static extern void gst_value_array_append_value (ref Gst.GLib.Value val, ref Gst.GLib.Value el);
  }
}
