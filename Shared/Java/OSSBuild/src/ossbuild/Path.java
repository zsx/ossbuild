
package ossbuild;

import java.io.File;

/**
 *
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
public class Path {
	//<editor-fold defaultstate="collapsed" desc="Constants">
	public static final String
		  directorySeparator = System.getProperty("file.separator")
		, pathSeparator = File.pathSeparator
	;

	public static final String
		  tempDirectory = clean(System.getProperty("java.io.tmpdir"))
		, homeDirectory = clean(System.getProperty("user.home"))
		, workingDirectory = clean(new File(".").getAbsolutePath())
	;
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Initialization">
	static {
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Public Static Methods">
	public static String nativeDirectorySeparator(final String path) {
		if (path == null)
			return StringUtil.empty;
		return path.replace("/", directorySeparator).replace("\\", directorySeparator);
	}

	public static String clean(final String path) {
		if (StringUtil.isNullOrEmpty(path))
			return StringUtil.empty;

		if (path.endsWith(directorySeparator))
			return path;

		if (path.endsWith("/") || path.endsWith("\\"))
			return path.substring(0, path.length() - 1) + directorySeparator;

		return path + directorySeparator;
	}

	public static File combine(final File parent, final String child) {
		return new File(parent, child);
	}

	public static File combine(final String parent, final String child) {
		return new File(parent, child);
	}
	//</editor-fold>
}
