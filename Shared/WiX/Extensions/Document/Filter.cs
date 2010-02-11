using System;
using System.Xml;
using System.Text;
using System.Collections.Generic;
using System.Text.RegularExpressions;

namespace OSSBuild.WiX {
	public class Filter {
		#region Constants
		private const string
			  ATTR_INCLUDE	= "Include"
			, ATTR_EXCLUDE	= "Exclude"
		;
		#endregion

		#region Variables
		private string include;
		private string exclude;
		private Regex regExpInclude;
		private Regex regExpExclude;
		#endregion

		#region Initialization
		public Filter(string Include, string Exclude) {
			init(Include, Exclude);
		}

		private void init(string Include, string Exclude) {
			this.regExpInclude = (string.IsNullOrEmpty(Include) ? null : new Regex("^" + Include + "$", RegexOptions.Compiled | RegexOptions.CultureInvariant | RegexOptions.IgnoreCase | RegexOptions.Singleline));
			this.regExpExclude = (string.IsNullOrEmpty(Exclude) ? null : new Regex("^" + Exclude + "$", RegexOptions.Compiled | RegexOptions.CultureInvariant | RegexOptions.IgnoreCase | RegexOptions.Singleline));
			this.include = Include;
			this.exclude = Exclude;

			if (regExpInclude == null && regExpExclude == null)
				throw new ArgumentNullException("Missing either an include or exclude filter");
		}
		#endregion

		#region Properties
		public string Include {
			get { return include; }
		}

		public string Exclude {
			get { return exclude; }
		}

		public Regex IncludeRegularExpression {
			get { return regExpInclude; }
		}

		public Regex ExcludeRegularExpression {
			get { return regExpExclude; }
		}
		#endregion

		#region Public Methods
		public bool ValidateInclude(string value) {
			if (regExpInclude == null)
				return true;
			return regExpInclude.IsMatch(value);
		}

		public bool ValidateExclude(string value) {
			if (regExpExclude == null)
				return false;
			return regExpExclude.IsMatch(value);
		}
		#endregion

		public static bool Validate(Filter[] filters, string value) {
			if (filters == null || filters.Length <= 0)
				return false;

			int includeCount = 0;
			int excludeCount = 0;
			int validIncludeCount = 0;
			
			foreach (Filter filter in filters) {
				if (filter.IncludeRegularExpression != null) {
					++includeCount;
					if (filter.ValidateInclude(value))
						++validIncludeCount;
				}
				if (filter.ExcludeRegularExpression != null) {
					++excludeCount;
					if (filter.ValidateExclude(value))
						return false;
				}
			}

			return (includeCount <= 0 || validIncludeCount > 0);
		}

		public static Filter[] Parse(XmlNode node) {
			//Find child filters
			XmlNamespaceManager nm = new XmlNamespaceManager(node.OwnerDocument.NameTable);
			nm.AddNamespace("ob", Namespace.OSSBuild);

			XmlNodeList filterNodes = node.SelectNodes("//ob:Filter", nm);
			List<Filter> filters = new List<Filter>(filterNodes.Count);

			foreach(XmlNode filterNode in filterNodes) {
				XmlAttributeCollection attributes = filterNode.Attributes;
				string include	= (attributes[ATTR_INCLUDE] != null ? attributes[ATTR_INCLUDE].Value : string.Empty);
				string exclude	= (attributes[ATTR_EXCLUDE] != null ? attributes[ATTR_EXCLUDE].Value : string.Empty);

				filters.Add(new Filter(include, exclude));
			}

			return filters.ToArray();
		}
	}
}
