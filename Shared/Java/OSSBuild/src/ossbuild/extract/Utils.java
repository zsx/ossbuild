package ossbuild.extract;

import java.io.File;
import java.net.URL;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.util.ArrayList;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.ThreadFactory;
import org.w3c.dom.Node;
import ossbuild.StringUtil;

/**
 * Utility class.
 *
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
class Utils {
	public static ExecutorService createPrivilegedExecutorService() {
		return Executors.newSingleThreadExecutor(createPrivilegedThreadFactory());
	}

	public static ThreadFactory createPrivilegedThreadFactory() {
		return AccessController.doPrivileged(new PrivilegedAction<ThreadFactory>() {
			public ThreadFactory run() {
				return Executors.privilegedThreadFactory();
			}
		});
	}

	//Thank you http://forums.sun.com/thread.jspa?threadID=341935
	public static Class[] getClasses(String pckgname) throws ClassNotFoundException {
		ArrayList<Class> classes = new ArrayList<Class>(3);
		
		// Get a File object for the package
		File directory = null;
		try {
			ClassLoader cld = Thread.currentThread().getContextClassLoader();
			if (cld == null)
				throw new ClassNotFoundException("Can't get class loader.");

			String path = pckgname.replace('.', '/');
			URL resource = cld.getResource(path);
			if (resource == null)
				throw new ClassNotFoundException("No resource for " + path);

			directory = new File(resource.getFile());
		} catch (NullPointerException x) {
			throw new ClassNotFoundException(pckgname + " (" + directory + ") does not appear to be a valid package");
		}

		if (directory.exists()) {
			// Get the list of the files contained in the package
			String[] files = directory.list();
			for (int i = 0; i < files.length; i++) {
				// we are only interested in .class files
				if (files[i].endsWith(".class")) {
					// removes the .class extension
					classes.add(Class.forName(pckgname + '.' + files[i].substring(0, files[i].length() - 6)));
				}
			}
		} else {
			throw new ClassNotFoundException(pckgname + " does not appear to be a valid package");
		}

		Class[] classesA = new Class[classes.size()];
		classes.toArray(classesA);
		return classesA;
	}

	public static boolean supportsSize(final Class Cls) {
		final ResourceProcessor info = resourceAnnotationForClass(Cls);
		if (info == null)
			return false;
		return info.supportsSize();
	}

	public static ResourceProcessor resourceAnnotationForClass(final Class Cls) {
		if (Cls == null)
			return null;

		return (ResourceProcessor)Cls.getAnnotation(ResourceProcessor.class);
	}

	public static boolean validateProcessor(final Class Cls) {
		//Extract info about this processor
		final ResourceProcessor info = resourceAnnotationForClass(Cls);
		if (info == null)
			return false;

		//Do we have a valid tag name?
		final String tagName = info.tagName();
		if (StringUtil.isNullOrEmpty(tagName))
			return false;

		//Does this class implement the desired interface?
		if (!IResourceProcessor.class.isAssignableFrom(Cls))
			return false;

		return true;
	}

	public static String expandVariables(String value) {
		return Variables.process(value);
	}

	public static String valueForAttribute(Node node, String name) {
		final Node attrib = node.getAttributes().getNamedItem(name);
		if (attrib == null || attrib.getNodeValue() == null)
			return StringUtil.empty;
		return expandVariables(attrib.getNodeValue());
	}

	public static String stringAttributeValue(String defaultValue, Node node, String name) {
		final String value = valueForAttribute(node, name);
		if (StringUtil.isNullOrEmpty(value))
			return defaultValue;
		return value;
	}

	public static boolean boolAttributeValue(boolean defaultValue, Node node, String name) {
		final String value = valueForAttribute(node, name);
		if (StringUtil.isNullOrEmpty(value))
			return defaultValue;
		return Boolean.parseBoolean(value);
	}

	public static File fileAttributeValue(String defaultValue, Node node, String name) {
		final String value = valueForAttribute(node, name);
		if (StringUtil.isNullOrEmpty(value))
			return new File(defaultValue);
		return new File(value);
	}

	public static File fileAttributeValue(File defaultValue, Node node, String name) {
		final String value = valueForAttribute(node, name);
		if (StringUtil.isNullOrEmpty(value))
			return defaultValue;
		return new File(value);
	}
}
