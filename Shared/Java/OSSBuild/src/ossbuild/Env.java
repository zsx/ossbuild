package ossbuild;

import com.sun.jna.Library;
import com.sun.jna.Native;

/**
 * Sets/unsets environment variables.
 * 
 * Courtesy of
 *	http://quirkygba.blogspot.com/2009/11/setting-environment-variables-in-java.html
 * 
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
public class Env {
	//<editor-fold defaultstate="collapsed" desc="JNA Library Declarations">
	interface EnvLibraryWindows extends Library {
		public String getenv(final String name);
		public int _putenv(final String name);
	}

	interface EnvLibraryUnix extends Library {
		public String getenv(final String name);
		public int setenv(final String name, final String value, final int overwrite);
		public int unsetenv(final String name);
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Variables">
	private static Library envlib;
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Initialization">
	static {
		switch(OS.getSystemOS()) {
			case Unix:
			case Mac:
				envlib = (Library)Native.loadLibrary("c", EnvLibraryUnix.class);
				break;
			case Windows:
				envlib = (Library)Native.loadLibrary("msvcrt", EnvLibraryWindows.class);
				break;
			default:
				break;
		}
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Public Static Methods">
	public static String getEnvironmentVariable(final String name) {
		if (envlib instanceof EnvLibraryUnix)
			return ((EnvLibraryUnix)envlib).getenv(name);
		else if (envlib instanceof EnvLibraryWindows)
			return ((EnvLibraryWindows)envlib).getenv(name);
		else
			throw new UnsatisfiedLinkError("Platform specific library for environment variable manipulation is unavailable");
	}

	public static boolean setEnvironmentVariable(final String name, final String value) {
		if (envlib instanceof EnvLibraryUnix)
			return (((EnvLibraryUnix)envlib).setenv(name, value, 1) == 0);
		else if (envlib instanceof EnvLibraryWindows)
			return (((EnvLibraryWindows)envlib)._putenv(name + "=" + value) == 0);
		else 
			throw new UnsatisfiedLinkError("Platform specific library for environment variable manipulation is unavailable");
	}

	public static boolean unsetEnvironmentVariable(final String name) {
		if (envlib instanceof EnvLibraryUnix)
			return (((EnvLibraryUnix)envlib).unsetenv(name) == 0);
		else if (envlib instanceof EnvLibraryWindows)
			return (((EnvLibraryWindows)envlib)._putenv(name + "=") == 0);
		else
			throw new UnsatisfiedLinkError("Platform specific library for environment variable manipulation is unavailable");
	}
	//</editor-fold>
}
