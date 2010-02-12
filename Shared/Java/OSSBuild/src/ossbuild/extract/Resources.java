
package ossbuild.extract;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map.Entry;
import java.util.concurrent.CancellationException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.RejectedExecutionException;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.ThreadPoolExecutor;
import javax.xml.namespace.NamespaceContext;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathException;
import javax.xml.xpath.XPathFactory;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;
import ossbuild.StringUtil;

/**
 * Reads a simple XML file that describes files to be extracted.
 *
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
public class Resources {
	//<editor-fold defaultstate="collapsed" desc="Constants">
	public static final String
		  NAMESPACE_PREFIX	= "OSSBuild"
		, NAMESPACE_URI		= "http://code.google.com/p/ossbuild/"
	;
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Variables">
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Initialization">
	public Resources(InputStream XMLData) throws XPathException, ParserConfigurationException, SAXException, IOException {
		init(XMLData);
	}

	protected void init(InputStream XMLData) throws XPathException, ParserConfigurationException, SAXException, IOException {
		final DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
		final DocumentBuilder builder = factory.newDocumentBuilder();
		final XPathFactory xpathFactory = XPathFactory.newInstance();
		final XPath xpath = xpathFactory.newXPath();

		//Creates a context that always has "ossbuild" as a prefix -- useful for xpath evaluations
		xpath.setNamespaceContext(new NamespaceContext() {
			private HashMap<String, String> namespaces = new HashMap<String, String>(1);

			{
				namespaces.put(NAMESPACE_PREFIX, NAMESPACE_URI);
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
		});

		read(xpath, builder.parse(XMLData));
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Helper">
	public ExecutorService createPrivilegedExecutorService() {
		return Executors.newSingleThreadExecutor(createPrivilegedThreadFactory());
	}

	public ThreadFactory createPrivilegedThreadFactory() {
		return AccessController.doPrivileged(new PrivilegedAction<ThreadFactory>() {
			public ThreadFactory run() {
				return Executors.privilegedThreadFactory();
			}
		});
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Public Methods">
	//<editor-fold defaultstate="collapsed" desc="Overloads">
	public Future Load(final IResourceCallback callback) {
		return Load(createPrivilegedExecutorService(), IResourceFilter.None, IResourceProgressListener.None, callback);
	}

	public Future Load(final IResourceProgressListener progress) {
		return Load(createPrivilegedExecutorService(), IResourceFilter.None, progress, IResourceCallback.None);
	}

	public Future Load(final IResourceFilter filter) {
		return Load(createPrivilegedExecutorService(), filter, IResourceProgressListener.None, IResourceCallback.None);
	}

	public Future Load(final IResourceFilter filter, final IResourceCallback callback) {
		return Load(createPrivilegedExecutorService(), filter, IResourceProgressListener.None, callback);
	}

	public Future Load(final IResourceProgressListener progress, final IResourceCallback callback) {
		return Load(createPrivilegedExecutorService(), IResourceFilter.None, progress, callback);
	}

	public Future Load(final IResourceFilter filter, final IResourceProgressListener progress, final IResourceCallback callback) {
		return Load(createPrivilegedExecutorService(), filter, progress, callback);
	}
	//</editor-fold>

	public Future Load(final ExecutorService executor, final IResourceFilter filter, final IResourceProgressListener progress, final IResourceCallback callback) {
		return executor.submit(new Runnable() {
			public void run() {
				try {
					//<editor-fold defaultstate="collapsed" desc="Make compiler happy">
					if (false)
						throw new InterruptedException();
					//</editor-fold>

					//<editor-fold defaultstate="collapsed" desc="Prepare">
					if (callback != null)
						callback.prepare(Resources.this);
					//</editor-fold>

					//Do the real work
					load(filter, progress);

					//<editor-fold defaultstate="collapsed" desc="Completed">
					if (callback != null)
						callback.completed(Resources.this);
					//</editor-fold>
				} catch(InterruptedException ie) {
					//<editor-fold defaultstate="collapsed" desc="Cancelled">
					if (callback != null)
						callback.cancelled(Resources.this);
					//</editor-fold>
				} catch(CancellationException ce) {
					//<editor-fold defaultstate="collapsed" desc="Cancelled">
					if (callback != null)
						callback.cancelled(Resources.this);
					//</editor-fold>
				} catch(RejectedExecutionException ree) {
					//<editor-fold defaultstate="collapsed" desc="Cancelled">
					if (callback != null)
						callback.cancelled(Resources.this);
					//</editor-fold>
				} catch(Throwable t) {
					//<editor-fold defaultstate="collapsed" desc="Error">
					if (callback != null)
						callback.error(Resources.this);
					//</editor-fold>
				}
			}
		});
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Public Static Methods">
	public static final Resources ReadFrom(String ResourceName) {
		return ReadFrom(Resources.class.getResourceAsStream(ResourceName));
	}

	public static final Resources ReadFrom(File XMLFile) {
		FileInputStream fis = null;
		try {
			return ReadFrom((fis = new FileInputStream(XMLFile)));
		} catch(Throwable t) {
			return null;
		} finally {
			try {
				if (fis != null)
					fis.close();
			} catch(IOException ie) {
			}
		}
	}

	public static final Resources ReadFrom(InputStream XMLData) {
		try {
			return new Resources(XMLData);
		} catch(Throwable t) {
			return null;
		}
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="The Meat">
	protected void load(final IResourceFilter filter, final IResourceProgressListener progress) throws Exception {
		//Please note that this is executed in a separate thread.
		//It can be cancelled or interrupted at any time.

		
	}

	protected void read(XPath xpath, Document document) throws XPathException  {
		Node node;
		NodeList lst;
		String nodeName;

		//Collapse whitespace nodes
		document.normalize();

		//Get the top-level document element, <Resources />
		final Element top = document.getDocumentElement();

		//Locate <Extract /> tags
		if ((lst = (NodeList)xpath.evaluate("//Resources/Extract", top, XPathConstants.NODESET)) == null || lst.getLength() <= 0)
			return;

		//Iterate over every <Extract /> tag
		for(int i = 0; i < lst.getLength() && (node = lst.item(i)) != null; ++i) {
			//Get the name and ensure that it's not empty
			if (StringUtil.isNullOrEmpty(nodeName = node.getNodeName()))
				continue;

			//Load an instance from our load manager
		}
	}
	//</editor-fold>
}
