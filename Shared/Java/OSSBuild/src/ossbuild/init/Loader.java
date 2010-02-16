
package ossbuild.init;

import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.List;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathException;
import javax.xml.xpath.XPathFactory;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;
import ossbuild.Namespaces;
import ossbuild.StringUtil;

/**
 *
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
public class Loader {
	//<editor-fold defaultstate="collapsed" desc="Constants">
	public static final String
		INIT_RESOURCE = "/resources/init.xml"
	;

	public static final String
		ATTRIBUTE_CLASS = "class"
	;
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Variables">
	private static boolean unloaded = true;
	private static boolean initialized = false;
	private static final Object lock = new Object();
	private static final List<LoaderInfo> loaders = new ArrayList<LoaderInfo>(2);
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Getters">
	public static Object getLock() {
		return lock;
	}

	public static boolean isInitialized() {
		synchronized(lock) {
			return initialized;
		}
	}

	public static boolean isUnloaded() {
		synchronized(lock) {
			return unloaded;
		}
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Helper Methods">
	protected static String valueForAttribute(Node node, String name) {
		final Node attrib = node.getAttributes().getNamedItem(name);
		if (attrib == null || attrib.getNodeValue() == null)
			return StringUtil.empty;
		return attrib.getNodeValue();
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Public Static Methods">
	public static boolean initialize() throws Throwable {
		synchronized(lock) {
			if (initialized)
				return true;

			initialized = true;
			unloaded = false;

			//Ensure that we unload when the application ends
			Runtime.getRuntime().addShutdownHook(new Thread() {
				@Override
				public void run() {
					Loader.unload();
				}
			});

			final String resource = (INIT_RESOURCE.startsWith("/") ? INIT_RESOURCE.substring(1) : INIT_RESOURCE);

			try {
				final DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
				final DocumentBuilder builder = factory.newDocumentBuilder();
				final XPathFactory xpathFactory = XPathFactory.newInstance();
				final XPath xpath = xpathFactory.newXPath();

				//Creates a context that always has "ossbuild" as a prefix -- useful for xpath evaluations
				xpath.setNamespaceContext(Namespaces.createNamespaceContext());

				//Cycle through every file at /resources/init.xml. There can be multiple ones in different
				//jars - using the class loader's getResources() allows us to access all of them.
				URL url;
				Enumeration<URL> e = Thread.currentThread().getContextClassLoader().getResources(resource);
				while(e.hasMoreElements()) {
					url = e.nextElement();

					Document document = null;
					LoaderInfo info = null;
					InputStream input = null;
					try {
						input = url.openStream();
						document = builder.parse(input);
					} catch (SAXException ex) {
						return false;
					} catch(IOException ie) {
						return false;
					} finally {
						try {
							if (input != null)
								input.close();
						} catch(IOException ie) {
						}
					}

					//Collapse whitespace nodes
					document.normalize();

					//Get the top-level document element, <System />
					final Element top = document.getDocumentElement();
					
					try {
						Node node;
						NodeList lst;

						//Locate <Load /> tags
						if ((lst = (NodeList)xpath.evaluate("//System/Load", top, XPathConstants.NODESET)) == null || lst.getLength() <= 0)
							continue;

						//Iterate over every <Load /> tag
						for(int i = 0; i < lst.getLength() && (node = lst.item(i)) != null; ++i) {

							//Examine the individual loader
							if ((info = readLoader(xpath, document, node)) != null)
								loaders.add(info);
						}
						
					} catch(XPathException t) {
						return false;
					}
				}

				//If we have a loader, then call its .load() method
				if (!loaders.isEmpty())
					for(LoaderInfo info : loaders)
						if (info != null)
							info.load();

				return true;
			} finally {
			}
		}
	}

	public static boolean unload() {
		synchronized(lock) {
			if (!initialized)
				return true;
			if (unloaded)
				return true;
			
			unloaded = true;
			initialized = false;

			try {
				for(LoaderInfo info : loaders)
					if (info != null)
						info.unload();
			} catch(Throwable t) {
				throw new RuntimeException(t);
			}
			return true;
		}
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="The Meat">
	private static LoaderInfo readLoader(final XPath xpath, final Document document, final Node node) throws XPathException {
		final String className = valueForAttribute(node, ATTRIBUTE_CLASS).trim();
		if (StringUtil.isNullOrEmpty(className))
			return null;

		try {
			final Class cls = Class.forName(className, true, Thread.currentThread().getContextClassLoader());
			if (!ISystemLoader.class.isAssignableFrom(cls))
				return null;

			final ISystemLoader instance = (ISystemLoader)cls.newInstance();

			return new LoaderInfo(cls, instance);
		} catch(Throwable t) {
			return null;
		}
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Helper Classes">
	private static class LoaderInfo {
		//<editor-fold defaultstate="collapsed" desc="Variables">
		private Class loaderCls;
		private ISystemLoader instance;
		//</editor-fold>

		//<editor-fold defaultstate="collapsed" desc="Initialization">
		public LoaderInfo(final Class cls, final ISystemLoader instance) {
			this.loaderCls = cls;
			this.instance = instance;
		}
		//</editor-fold>

		//<editor-fold defaultstate="collapsed" desc="Getters">
		public Class getLoaderClass() {
			return loaderCls;
		}

		public ISystemLoader getInstance() {
			return instance;
		}
		//</editor-fold>

		//<editor-fold defaultstate="collapsed" desc="Public Methods">
		public void load() throws Throwable {
			if (instance != null)
				instance.load();
		}

		public void unload() throws Throwable {
			if (instance != null)
				instance.unload();
		}
		//</editor-fold>
	}
	//</editor-fold>
}
