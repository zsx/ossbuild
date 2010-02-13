
package ossbuild.extract;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * Marks a class as something that can process and load resources.
 *
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
@Documented
@Target(ElementType.TYPE)
@Retention(RetentionPolicy.RUNTIME)
public @interface ResourceProcessor {
	String tagName();
	boolean supportsSize() default false;
}
