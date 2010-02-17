
package ossbuild.extract;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.List;
import java.util.logging.Level;
import java.util.logging.Logger;
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
import ossbuild.Namespaces;
import ossbuild.StringUtil;

/**
 * Maintains a list of available resource processors and is responsible for
 * creating new ones from a
 *
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
public class ResourceProcessorFactory {
	//<editor-fold defaultstate="collapsed" desc="Constants">
	public static final String
		PROCESSORS_RESOURCE = "/resources/extract/processors.xml"
	;

	public static final String
		ATTRIBUTE_CLASS = "class"
	;

	public static final ResourceProcessorFactory
		DEFAULT_INSTANCE
	;
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Variables">
	protected static Map<String, Class> defaultProcessors;
	protected Map<String, Class> processors;
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Initialization">
	static {
		initializeDefaultProcessors();
		DEFAULT_INSTANCE = newInstance();
	}

	private static void initializeDefaultProcessors() {
		defaultProcessors = new HashMap<String, Class>(3, 0.5f);

		//Load every class in /resources/processors.xml
		final Class[] classes = readDefaultProcessors();
		if (classes == null || classes.length <= 0)
				return;

		addDefaultProcessors(classes);
	}

	public ResourceProcessorFactory() {
		init(true);
	}

	public ResourceProcessorFactory(final boolean addDefaultProcessors) {
		init(addDefaultProcessors);
	}

	private void init(final boolean addDefaultProcessors) {
		this.processors = new HashMap<String, Class>(3, 0.5f);

		if (addDefaultProcessors && !defaultProcessors.isEmpty())
			processors.putAll(defaultProcessors);
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Getters">
	public static final Map<String, Class> getDefaultProcessors() {
		return Collections.unmodifiableMap(defaultProcessors);
	}

	public final Map<String, Class> getProcessors() {
		return Collections.unmodifiableMap(processors);
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Helper Methods">
	public static ResourceProcessor resourceAnnotationForClass(final Class Cls) {
		return ResourceUtils.resourceAnnotationForClass(Cls);
	}

	public static boolean validateProcessor(final Class Cls) {
		return ResourceUtils.validateProcessor(Cls);
	}

	protected static String valueForAttribute(Node node, String name) {
		final Node attrib = node.getAttributes().getNamedItem(name);
		if (attrib == null || attrib.getNodeValue() == null)
			return StringUtil.empty;
		return attrib.getNodeValue();
	}
	
	private static boolean addProcessorToMap(final Map<String, Class> Map, Class Cls) {
		if (Cls == null || Map.containsValue(Cls))
			return false;

		//Extract info about this processor
		final ResourceProcessor info = resourceAnnotationForClass(Cls);
		if (info == null)
			return false;

		//Do we have a valid tag name?
		final String tagName = info.tagName();
		if (StringUtil.isNullOrEmpty(tagName))
			throw new NullPointerException("Missing tag name in " + ResourceProcessor.class.getSimpleName() + " annotation for " + Cls.getSimpleName());

		//Does this class implement the desired interface?
		if (!IResourceProcessor.class.isAssignableFrom(Cls))
			return false;
		
		Map.put(tagName, Cls);
		return true;
	}
	
	private static boolean removeProcessorFromMap(final Map<String, Class> Map, Class Cls) {
		if (Cls == null || Map == null || Map.isEmpty() || !Map.containsValue(Cls))
			return false;

		for(Map.Entry<String, Class> entry : Map.entrySet()) {
			if (Cls.equals(entry.getValue())) {
				Map.remove(entry.getKey());
				return true;
			}
		}
		
		return false;
	}

	private static boolean clearProcessorsFromMap(final Map<String, Class> Map) {
		Map.clear();
		return true;
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Public Methods">
	public boolean addProcessors(final Class... Classes) {
		boolean ret = true;
		for(Class cls : Classes)
			ret = ret && addProcessorToMap(processors, cls);
		return ret;
	}

	public boolean removeProcessors(final Class... Classes) {
		boolean ret = true;
		for(Class cls : Classes)
			ret = ret && removeProcessorFromMap(processors, cls);
		return ret;
	}

	public boolean clearProcessors() {
		return clearProcessorsFromMap(processors);
	}

	public String tagNameForProcessor(final Class Cls) {
		if (Cls == null || processors.isEmpty())
			return StringUtil.empty;

		final ResourceProcessor info = resourceAnnotationForClass(Cls);
		if (info == null || StringUtil.isNullOrEmpty(info.tagName()))
			return StringUtil.empty;
		
		return info.tagName();
	}

	public IResourceProcessor createProcessor(final String TagName) {
		final Class cls;
		if (StringUtil.isNullOrEmpty(TagName) || processors.isEmpty() || !processors.containsKey(TagName) || (cls = processors.get(TagName)) == null)
			return null;
		return newProcessor(cls);
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Public Static Methods">
	public static ResourceProcessorFactory newInstance() {
		return new ResourceProcessorFactory();
	}

	public static ResourceProcessorFactory newInstance(final boolean AddDefaultProcessors) {
		return new ResourceProcessorFactory(AddDefaultProcessors);
	}

	public static ResourceProcessorFactory newInstance(final Class... AdditionalProcessors) {
		final ResourceProcessorFactory factory = newInstance();
		factory.addProcessors(AdditionalProcessors);
		return factory;
	}

	public static ResourceProcessorFactory newInstance(final boolean AddDefaultProcessors, final Class... AdditionalProcessors) {
		final ResourceProcessorFactory factory = newInstance(AddDefaultProcessors);
		factory.addProcessors(AdditionalProcessors);
		return factory;
	}

	public static boolean addDefaultProcessors(Class... Classes) {
		boolean ret = true;
		for(Class cls : Classes)
			ret = ret && addProcessorToMap(defaultProcessors, cls);
		return ret;
	}

	public static boolean removeDefaultProcessors(Class... Classes) {
		boolean ret = true;
		for(Class cls : Classes)
			ret = ret && removeProcessorFromMap(defaultProcessors, cls);
		return ret;
	}

	public static boolean clearDefaultProcessors() {
		return clearProcessorsFromMap(defaultProcessors);
	}

	public static IResourceProcessor newProcessor(final Class Cls) {
		if (Cls == null)
			return null;

		try {
			return (IResourceProcessor)Cls.newInstance();
		} catch(Throwable t) {
			return null;
		}
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="The Meat">
	protected static Class[] readDefaultProcessors() {
		final List<Class> clsList = new ArrayList<Class>(2);
		final String resource = (PROCESSORS_RESOURCE.startsWith("/") ? PROCESSORS_RESOURCE.substring(1) : PROCESSORS_RESOURCE);

		try {
			//<editor-fold defaultstate="collapsed" desc="Prepare document">
			final DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
			final DocumentBuilder builder = factory.newDocumentBuilder();
			final XPathFactory xpathFactory = XPathFactory.newInstance();
			final XPath xpath = xpathFactory.newXPath();

			//Creates a context that always has "ossbuild" as a prefix -- useful for xpath evaluations
			xpath.setNamespaceContext(Namespaces.createNamespaceContext());
			//</editor-fold>

			//Cycle through every file at /resources/init.xml. There can be multiple ones in different
			//jars - using the class loader's getResources() allows us to access all of them.
			URL url;
			Enumeration<URL> e = Thread.currentThread().getContextClassLoader().getResources(resource);
			while(e.hasMoreElements()) {
				url = e.nextElement();

				//<editor-fold defaultstate="collapsed" desc="Build document">
				Document document = null;
				Class cls = null;
				InputStream input = null;
				try {
					input = url.openStream();
					document = builder.parse(input);
				} catch (SAXException ex) {
					return null;
				} catch(IOException ie) {
					return null;
				} finally {
					try {
						if (input != null)
							input.close();
					} catch(IOException ie) {
					}
				}

				//Collapse whitespace nodes
				document.normalize();

				//Get the top-level document element, <ResourceProcessors />
				final Element top = document.getDocumentElement();
				//</editor-fold>

				try {
					Node node;
					NodeList lst;

					//Locate <Load /> tags
					if ((lst = (NodeList)xpath.evaluate("//ResourceProcessors/Load", top, XPathConstants.NODESET)) == null || lst.getLength() <= 0)
						continue;

					//Iterate over every <Load /> tag
					for(int i = 0; i < lst.getLength() && (node = lst.item(i)) != null; ++i) {

						//Examine the individual loader
						if ((cls = readProcessor(xpath, document, node)) != null)
							clsList.add(cls);
					}
				} catch(XPathException t) {
					return null;
				}
			}

			return clsList.toArray(new Class[clsList.size()]);
		} catch (IOException ex) {
			return null;
		} catch (ParserConfigurationException ex) {
			return null;
		} finally {
		}
	}

	private static Class readProcessor(final XPath xpath, final Document document, final Node node) throws XPathException {
		final String className = valueForAttribute(node, ATTRIBUTE_CLASS).trim();
		if (StringUtil.isNullOrEmpty(className))
			return null;

		try {
			final Class cls = Class.forName(className, true, Thread.currentThread().getContextClassLoader());
			if (!IResourceProcessor.class.isAssignableFrom(cls))
				return null;

			return cls;
		} catch(Throwable t) {
			return null;
		}
	}
	//</editor-fold>
}
