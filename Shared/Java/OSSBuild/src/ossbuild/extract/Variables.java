
package ossbuild.extract;

import java.util.HashMap;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import ossbuild.Path;
import ossbuild.StringUtil;
import ossbuild.Sys;

/**
 * Holds variables and their values.
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
public class Variables {
	//<editor-fold defaultstate="collapsed" desc="Constants">
	public static final String
		  VAR_PID               = "pid"
		, VAR_CWD               = "cwd"
		, VAR_TMP               = "tmp"
		, VAR_HOME              = "home"
		, VAR_PATH				= "env_PATH"
		, VAR_PATH_SEP          = "pathSep"
		, VAR_ORIG_CWD          = "origcwd"
		, VAR_ORIG_ENV_PATH		= "origenv_PATH"
	;

	private static final Pattern
		REGEX_VAR = Pattern.compile("\\$\\{([^\\}]*)\\}", Pattern.CASE_INSENSITIVE | Pattern.DOTALL | Pattern.CANON_EQ);
	;
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Variables">
	private static Map<String, String> vars;
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Initialization">
	static {
		vars = new HashMap<String, String>(3, 0.5f);

		saveVariable(VAR_ORIG_CWD, Path.workingDirectory);
		saveVariable(VAR_ORIG_ENV_PATH, Sys.getPath());

		saveVariable(VAR_PID, Sys.getPID() + "");
		saveVariable(VAR_CWD, Path.workingDirectory);
		saveVariable(VAR_TMP, Path.tempDirectory);
		saveVariable(VAR_HOME, Path.homeDirectory);
		saveVariable(VAR_PATH, Sys.getPath());
		saveVariable(VAR_PATH_SEP, Path.pathSeparator);
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Public Static Methods">
	public static boolean saveVariable(String Name, String Value) {
		if (Value == null)
			Value = StringUtil.empty;
		vars.put(Name, Matcher.quoteReplacement(Value));
		return true;
	}

	public static boolean removeVariable(String Name) {
		if (vars.isEmpty() || !vars.containsKey(Name))
			return false;
		vars.remove(Name);
		return true;
	}

	public static boolean clearVariables() {
		vars.clear();
		return true;
	}

	public static String process(String value) {
		final Matcher m = REGEX_VAR.matcher(value);
		if (!m.find())
			return value;

		//We found a variable, so loop through all the matches
		//and do our find/replace.
		final StringBuffer sb = new StringBuffer(128);
		do {
			if (m.groupCount() < 1)
				continue;

			String varName = m.group(1);
			String varValue = vars.get(varName);

			m.appendReplacement(sb, varValue);
		} while(m.find());
		m.appendTail(sb);
		
		return sb.toString();
	}
	//</editor-fold>
}
