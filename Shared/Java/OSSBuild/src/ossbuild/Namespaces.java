
package ossbuild;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map.Entry;
import javax.xml.namespace.NamespaceContext;

/**
 *
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
public class Namespaces {
	//<editor-fold defaultstate="collapsed" desc="Constants">
	public static final String
		  DEFAULT_NAMESPACE_PREFIX	= "OSSBuild"
		, DEFAULT_NAMESPACE_URI		= "http://code.google.com/p/ossbuild/"
	;
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Public Static Methods">
	public static NamespaceContext createNamespaceContext() {
		return new NamespaceContext() {
			private HashMap<String, String> namespaces = new HashMap<String, String>(1);

			{
				namespaces.put(DEFAULT_NAMESPACE_PREFIX, DEFAULT_NAMESPACE_URI);
			}

			public String getNamespaceURI(String prefix) {
				if (namespaces.isEmpty() || StringUtil.isNullOrEmpty(prefix) || !namespaces.containsKey(prefix))
					return StringUtil.empty;
				return namespaces.get(prefix);
			}

			public Iterator getPrefixes(String namespaceURI) {
				return namespaces.values().iterator();
			}

			public String getPrefix(String namespaceURI) {
				if (StringUtil.isNullOrEmpty(namespaceURI))
					return StringUtil.empty;

				for(Entry<String, String> entry : namespaces.entrySet())
					if (namespaceURI.equals(entry.getValue()))
						return entry.getKey();

				return StringUtil.empty;
			}
		};
	}
	//</editor-fold>
}
