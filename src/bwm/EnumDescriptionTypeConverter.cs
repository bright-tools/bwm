using System;
using System.ComponentModel;
using System.Reflection;

namespace bwm
{
    /* Originally taken from http://brianlagunas.com/a-better-way-to-data-bind-enums-in-wpf/
     * & subsequently expanded for use in BWM */

    public class EnumDescriptionTypeConverter : EnumConverter
    {
        public EnumDescriptionTypeConverter(Type type)
            : base(type)
        {
        }
        public override object ConvertTo(ITypeDescriptorContext context, System.Globalization.CultureInfo culture, object value, Type destinationType)
        {
            if (destinationType == typeof(string))
            {
                if (value != null)
                {
                    FieldInfo fi = value.GetType().GetField(value.ToString());
                    if (fi != null)
                    {
                        var attributes = (DescriptionAttribute[])fi.GetCustomAttributes(typeof(DescriptionAttribute), false);
                        return ((attributes.Length > 0) && (!String.IsNullOrEmpty(attributes[0].Description))) ? attributes[0].Description : value.ToString();
                    }
                }

                return string.Empty;
            }

            return base.ConvertTo(context, culture, value, destinationType);
        }
        public override object ConvertFrom(ITypeDescriptorContext context,
                                           System.Globalization.CultureInfo culture, object value)
        {
            if (value is string)
            {
                foreach( FieldInfo fi in this.EnumType.GetFields())
                {
                    var attributes = (DescriptionAttribute[])fi.GetCustomAttributes(typeof(DescriptionAttribute), false);
                    if ((attributes.Length > 0) && (value.ToString().Equals(attributes[0].Description)))
                    {
                        return fi.GetValue(null);
                    }
                }
                return Enum.ToObject(this.EnumType, MouseButton.LeftButton);
            }

            return base.ConvertFrom(context, culture, value);
        }
    }
}
