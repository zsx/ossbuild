
package ossbuild.extract;

import java.util.Map;

/**
 * Holds and processes variables.
 * 
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
public interface IVariableProcessor {
	boolean addVariables(final Map<String, String> Variables);
	boolean saveVariable(final String Name, final String Value);
	boolean removeVariable(final String Name);
	boolean clearVariables();

	String findValue(final String Name);
	String process(final String Value);
}
