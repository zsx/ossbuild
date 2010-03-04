
package ossbuild.extract;

/**
 * A variable was reference that has not been defined.
 * 
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
public class MissingVariableException extends RuntimeException {
    public MissingVariableException() {
    }

    public MissingVariableException(String message) {
        super(message);
    }

    public MissingVariableException(Throwable cause) {
        super(cause);
    }

    public MissingVariableException(String message, Throwable cause) {
        super(message, cause);
    }
}
