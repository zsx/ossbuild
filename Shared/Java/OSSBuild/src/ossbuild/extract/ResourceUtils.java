package ossbuild.extract;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.URL;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.util.ArrayList;
import java.util.Date;
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
public class ResourceUtils {
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

	public static ExecutorService createUnprivilegedExecutorService() {
		return Executors.newSingleThreadExecutor(createUnprivilegedThreadFactory());
	}

	public static ThreadFactory createUnprivilegedThreadFactory() {
		return Executors.defaultThreadFactory();
	}

	//Thank you http://forums.sun.com/thread.jspa?threadID=341935
	@Deprecated
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

	public static final boolean deleteDirectory(File path) {
		if (path == null)
			return false;
		if (path.exists()) {
			final File[] files = path.listFiles();
			for(int i = 0; i < files.length; ++i) {
				if (files[i].isDirectory())
					deleteDirectory(files[i]);
				else
					files[i].delete();
			}
		}
		return path.delete();
	}

	public static boolean supportsSize(final Class Cls) {
		final ResourceProcessor info = resourceAnnotationForClass(Cls);
		if (info == null)
			return false;
		return info.supportsSize();
	}

	@SuppressWarnings("unchecked")
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

	public static boolean attemptLibraryLoad(String libraryPath) {
		return (com.sun.jna.NativeLibrary.getInstance(libraryPath) != null);
	}

	public static boolean attemptSystemLibraryLoad(String libraryName) {
		try {
			return (com.sun.jna.NativeLibrary.getInstance(libraryName) != null);
		} catch(Throwable t) {
			return false;
		}
	}

	public static long sizeFromResource(String fullResourceName) {
		if (StringUtil.isNullOrEmpty(fullResourceName))
			return 0L;
		final URL url = ResourceUtils.class.getResource(fullResourceName);
		if (url == null)
			return 0L;
		try {
			return url.openConnection().getContentLength();
		} catch(IOException ie) {
			return 0L;
		}
	}

	public static long lastModifiedFromResource(String fullResourceName) {
		if (StringUtil.isNullOrEmpty(fullResourceName))
			return 0L;
		final URL url = ResourceUtils.class.getResource(fullResourceName);
		if (url == null)
			return 0L;
		try {
			return url.openConnection().getLastModified();
		} catch(IOException ie) {
			return 0L;
		}
	}

	public static boolean saveLastModified(File destination, long lastModified) {
		try {
			if (lastModified > 0L)
				destination.setLastModified(lastModified);
			return true;
		} catch(Throwable t) {
			return false;
		}
	}

	public static boolean extractResource(String fullResourceName, File destination, boolean isTransient) {
		//If the destination exists and it's last modified date/time is older than now,
		//then we can safely skip it. Otherwise, it must be replaced or created.
		long lastModified = ResourceUtils.lastModifiedFromResource(fullResourceName);
		if (destination.exists() && isTransient) {
			if (lastModified <= 0L)
				return false;
			if (destination.lastModified() >= lastModified)
				return true;
		}

		//Make any necessary directories
		final File parentDir = destination.getParentFile();
		if (!parentDir.exists())
			parentDir.mkdirs();
		if (!parentDir.exists() || !parentDir.isDirectory())
			return false;

		final byte[] buffer;
		InputStream input = null;
		OutputStream output = null;
		int read = 0;

		try {
			
			input = ResourceUtils.class.getResourceAsStream(fullResourceName);
			if (input == null)
				return false;

			buffer = new byte[4096];
			output = new FileOutputStream(destination);

			while((read = input.read(buffer, 0, buffer.length)) >= 0) {
				output.write(buffer, 0, read);
			}

			output.flush();
			return true;
		} catch(IOException ie) {
			return false;
		} finally {
			try {
				if (input != null)
					input.close();
			} catch(IOException ie) {
			}

			try {
				if (output != null)
					output.close();
			} catch(IOException ie) {
			}

			saveLastModified(destination, lastModified);
		}
	}
}
