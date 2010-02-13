
package ossbuild.extract;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import org.w3c.dom.Node;
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
		DEFAULT_PROCESSOR_PACKAGE = ResourceProcessorFactory.class.getPackage().getName() + ".processors"
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

		//Load every class under
		final Class[] classes;
		try {
			classes = Utils.getClasses(DEFAULT_PROCESSOR_PACKAGE);
			if (classes == null || classes.length <= 0)
				return;
		} catch(ClassNotFoundException ce) {
			throw new RuntimeException("Unable to locate resource processors", ce);
		}

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
		return Utils.resourceAnnotationForClass(Cls);
	}

	public static boolean validateProcessor(final Class Cls) {
		return Utils.validateProcessor(Cls);
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
}
