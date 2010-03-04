
package ossbuild.extract;

import java.util.HashMap;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import ossbuild.StringUtil;

/**
 * Holds variables, their values, and processes strings replacing variables with.
 *
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
public class DefaultVariableProcessor implements IVariableProcessor {
	//<editor-fold defaultstate="collapsed" desc="Constants">
	protected static final Pattern
		REGEX_VAR = VariableProcessorFactory.REGEX_VAR
	;
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Variables">
	private Map<String, String> vars = new HashMap<String, String>(8, 0.5f);
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Initialization">
	public DefaultVariableProcessor() {
		init();
	}

	protected void init() {
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Public Methods">
	@Override
	public boolean addVariables(final Map<String, String> Variables) {
		if (Variables == null || Variables.isEmpty())
			return true;
		for(Map.Entry<String, String> e : Variables.entrySet())
			vars.put(e.getKey(), e.getValue());
		return true;
	}

	@Override
	public boolean saveVariable(final String Name, String Value) {
		if (Value == null)
			Value = StringUtil.empty;
		vars.put(Name, Matcher.quoteReplacement(Value));
		return true;
	}

	@Override
	public boolean removeVariable(final String Name) {
		if (vars.isEmpty() || !vars.containsKey(Name))
			return false;
		vars.remove(Name);
		return true;
	}

	@Override
	public boolean clearVariables() {
		vars.clear();
		return true;
	}

	@Override
	public String findValue(final String Name) {
		if (vars.isEmpty() || !vars.containsKey(Name))
			return null;
		return vars.get(Name);
	}

	@Override
	public String process(final String value) {
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
