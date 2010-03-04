package ossbuild;

import com.sun.jna.Library;
import com.sun.jna.Native;
import ossbuild.platform.Win32Errors;
import ossbuild.platform.Win32Library;

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
	interface EnvLibraryWindows extends Win32Library {
		//<editor-fold defaultstate="collapsed" desc="Constants">
		public static final String
			LIB_NAME = "msvcrt"
		;

		public static final EnvLibraryWindows
			INSTANCE = (EnvLibraryWindows)Native.loadLibrary(LIB_NAME, EnvLibraryWindows.class)
		;
		//</editor-fold>

		public String getenv(final String name);
		public int _putenv(final String name);
	}

	interface EnvLibraryUnix extends Library {
		//<editor-fold defaultstate="collapsed" desc="Constants">
		public static final String
			LIB_NAME = "c"
		;

		public static final EnvLibraryUnix
			INSTANCE = (EnvLibraryUnix)Native.loadLibrary(LIB_NAME, EnvLibraryUnix.class)
		;
		//</editor-fold>

		public String getenv(final String name);
		public int setenv(final String name, final String value, final int overwrite);
		public int unsetenv(final String name);
		public int chdir(final String path);
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Constants">
	public static final String
		PATH = "PATH"
	;
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Variables">
	private static Library envlib;
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Initialization">
	static {
		switch(OS.getSystemOSFamily()) {
			case Unix:
			case Mac:
				envlib = EnvLibraryUnix.INSTANCE;
				break;
			case Windows:
				envlib = EnvLibraryWindows.INSTANCE;
				break;
			default:
				break;
		}
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Public Static Methods">
	public static String getPath() {
		return getEnvironmentVariable(PATH);
	}

	public static String getEnvironmentVariable(final String name) {
		if (envlib instanceof EnvLibraryUnix)
			return EnvLibraryUnix.INSTANCE.getenv(name);
		else if (envlib instanceof EnvLibraryWindows)
			return EnvLibraryWindows.INSTANCE.getenv(name);
		else
			throw new UnsatisfiedLinkError("Platform specific library for environment variable manipulation is unavailable");
	}

	public static boolean setEnvironmentVariable(final String name, final String value) {
		if (envlib instanceof EnvLibraryUnix)
			return (EnvLibraryUnix.INSTANCE.setenv(name, value, 1) == 0);
		else if (envlib instanceof EnvLibraryWindows)
			return (EnvLibraryWindows.INSTANCE._putenv(name + "=" + value) == 0);
		else 
			throw new UnsatisfiedLinkError("Platform specific library for environment variable manipulation is unavailable");
	}

	public static boolean unsetEnvironmentVariable(final String name) {
		if (envlib instanceof EnvLibraryUnix)
			return (EnvLibraryUnix.INSTANCE.unsetenv(name) == 0);
		else if (envlib instanceof EnvLibraryWindows)
			return (EnvLibraryWindows.INSTANCE._putenv(name + "=") == 0);
		else
			throw new UnsatisfiedLinkError("Platform specific library for environment variable manipulation is unavailable");
	}
	//</editor-fold>
}
