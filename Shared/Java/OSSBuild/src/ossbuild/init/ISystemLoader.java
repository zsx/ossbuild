
package ossbuild.init;

/**
 *
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
public interface ISystemLoader {
	void load() throws Throwable;
	void unload() throws Throwable;
}
