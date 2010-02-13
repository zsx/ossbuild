
package ossbuild;

/**
 * Gathers information about operating systems and the one we're hosted on.
 *
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
public enum OS {
	   Unknown(
		  StringUtil.empty
	), Windows(
		  "windows"

		, "Windows 95"
		, "Windows 98"
		, "Windows Me"
		, "Windows NT"
		, "Windows 2000"
		, "Windows XP"
		, "Windows 2003"
		, "Windows 2008"
		, "Windows Vista"
		, "Windows 7"
		, "Windows CE"
		, "OS/2"
	), Mac(
		  "osx"

		, "Mac OS"
		, "Mac OS X"
	), Unix(
		  "unix"

		, "Linux"
		, "MPE/iX"
		, "HP-UX"
		, "AIX"
		, "FreeBSD"
		, "Irix"
		, "OS/390"
		, "Digital Unix"
		, "NetWare 4.11"
		, "OSF1"
		, "SunOS"
	), Solaris(
		  "solaris"

		, "Solaris"
	), VMS(
		  "vms"
		
		, "OpenVMS"
	)
	;

	//<editor-fold defaultstate="collapsed" desc="Constants">
	public static final String
		NAME = System.getProperty("os.name")
	;
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Variables">
	private static OS systemOS;
	private String[] variants;
	private String platformPartName;
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Initialization">
	static {
		systemOS = fromName(NAME);
	}

	OS(final String PlatformPartName, final String...Variations) {
		this.variants = Variations;
		this.platformPartName = PlatformPartName;
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Getters">
	public static String getSystemOSName() {
		return NAME;
	}

	public static OS getSystemOS() {
		return systemOS;
	}

	public String[] getVariants() {
		return variants;
	}

	public String getPlatformPartName() {
		return platformPartName;
	}

	public boolean isPOSIX() {
		return isPOSIX(this);
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Public Static Methods">
	public static OS fromName(final String Name) {
		if (StringUtil.isNullOrEmpty(Name))
			return OS.Unknown;

		for(OS os : OS.values()) {
			for(String variant : os.variants)
				if (variant.equalsIgnoreCase(Name))
					return os;
		}

		final String lower = Name.toLowerCase();
		if (lower.contains("win"))
			return OS.Windows;
		else if (lower.contains("mac"))
			return OS.Mac;
		else if (lower.contains("nix") || lower.contains("nux"))
			return OS.Unix;
		else if (lower.contains("vms"))
			return OS.VMS;
		else if (lower.contains("solaris"))
			return OS.Solaris;
		else
			return OS.Unknown;
	}

	public static boolean isPOSIX(final OS OS) {
		switch(OS) {
			case Unix:
			case Mac:
			case Solaris:
				return true;
			default:
				return false;
		}
	}
	//</editor-fold>
}
