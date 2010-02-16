
package ossbuild.extract;

/**
 * Exception regarding a missing resource for extraction and/or loading.
 * 
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
public class MissingResourceException extends RuntimeException {
    public MissingResourceException() {
    }

    public MissingResourceException(String message) {
        super(message);
    }

    public MissingResourceException(Throwable cause) {
        super(cause);
    }

    public MissingResourceException(String message, Throwable cause) {
        super(message, cause);
    }
}
