
package ossbuild.extract;

import java.util.HashMap;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import ossbuild.Path;
import ossbuild.StringUtil;
import ossbuild.Sys;

/**
 * Factory for creating variable processors that evaluate variables when 
 * extracting resources.
 * 
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
public class VariableProcessorFactory {
	//<editor-fold defaultstate="collapsed" desc="Constants">
	public static final String
		  SYSTEM_PROPERTY_NAME_DEFAULT_VARIABLE_PROCESSOR_CLASS = "resources.extract.defaultProcessorClass"
	;

	public static final String
		  VAR_PID               = "pid"
		, VAR_CWD               = "cwd"
		, VAR_TMP               = "tmp"
		, VAR_HOME              = "home"
		, VAR_PATH				= "env_PATH"
		, VAR_PATH_SEP          = "pathSep"
		, VAR_ORIG_CWD          = "origcwd"
		, VAR_ORIG_ENV_PATH		= "origenv_PATH"

		, VAR_DIR               = "dir"
		, VAR_PKG               = "pkg"
	;

	public static final Pattern
		  REGEX_VAR = Pattern.compile("\\$\\{([^\\}]*)\\}", Pattern.CASE_INSENSITIVE | Pattern.DOTALL | Pattern.CANON_EQ)
	;
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Variables">
	private static Map<String, String> defaultVars;
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Initialization">
	static {
		defaultVars = new HashMap<String, String>(8, 0.5f);

		saveDefaultVariable(VAR_ORIG_CWD, Path.workingDirectory);
		saveDefaultVariable(VAR_ORIG_ENV_PATH, Sys.getPath());

		saveDefaultVariable(VAR_PID, Sys.getPID() + "");
		saveDefaultVariable(VAR_CWD, Path.workingDirectory);
		saveDefaultVariable(VAR_TMP, Path.tempDirectory);
		saveDefaultVariable(VAR_HOME, Path.homeDirectory);
		saveDefaultVariable(VAR_PATH, Sys.getPath());
		saveDefaultVariable(VAR_PATH_SEP, Path.pathSeparator);
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Public Static Methods">
	public static boolean saveDefaultVariable(final String Name, String Value) {
		if (Value == null)
			Value = StringUtil.empty;
		defaultVars.put(Name, Matcher.quoteReplacement(Value));
		return true;
	}

	public static boolean removeDefaultVariable(final String Name) {
		if (defaultVars.isEmpty() || !defaultVars.containsKey(Name))
			return false;
		defaultVars.remove(Name);
		return true;
	}

	public static boolean clearDefaultVariables() {
		defaultVars.clear();
		return true;
	}

	public static IVariableProcessor newInstance() {
		return newInstance(true);
	}

	public static IVariableProcessor newInstance(final boolean AddDefaultVariables) {
		return newInstance(AddDefaultVariables, System.getProperty(SYSTEM_PROPERTY_NAME_DEFAULT_VARIABLE_PROCESSOR_CLASS, DefaultVariableProcessor.class.getName()));
	}

	public static IVariableProcessor newInstance(final boolean AddDefaultVariables, final String VariableProcessorClassName) {
		if (StringUtil.isNullOrEmpty(VariableProcessorClassName))
			throw new NullPointerException("Missing parameter");

		final Class cls;
		try {
			cls = Class.forName(VariableProcessorClassName, true, Thread.currentThread().getContextClassLoader());
		} catch(Throwable t) {
			throw new IllegalArgumentException("Unable to locate the class " + VariableProcessorClassName + ". Please be sure the current thread's class loader can find and initialize it.");
		}
		
		return newInstance(AddDefaultVariables, cls);
	}

	public static IVariableProcessor newInstance(final boolean AddDefaultVariables, final Class VariableProcessorClass) {
		if (VariableProcessorClass == null)
			throw new NullPointerException("Missing parameter");
		if (!IVariableProcessor.class.isAssignableFrom(VariableProcessorClass))
			throw new IllegalArgumentException("Invalid class: " + VariableProcessorClass.getSimpleName() + ". It must implement the " + IVariableProcessor.class.getSimpleName() + " interface.");

		final IVariableProcessor vp;
		try {
			vp = (IVariableProcessor)VariableProcessorClass.newInstance();
		} catch(Throwable t) {
			throw new IllegalArgumentException("Unable to create a new instance of " + VariableProcessorClass.getSimpleName() + ". Please be sure it has a default constructor.");
		}

		if (AddDefaultVariables)
			if (!vp.addVariables(defaultVars))
				throw new IllegalStateException("Unable to add default variables to " + VariableProcessorClass.getSimpleName());
		
		return vp;
	}
	//</editor-fold>
}
