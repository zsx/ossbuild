
package ossbuild;

/**
 * Gathers information about operating systems and the one we're hosted on.
 *
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
public enum OS {
	  Unknown        (OSFamily.Unknown, StringUtil.empty)

	, Windows95      (OSFamily.Windows, "windows_95",    "Windows 95")
	, Windows98      (OSFamily.Windows, "windows_98",    "Windows 98")
	, WindowsMe      (OSFamily.Windows, "windows_me",    "Windows Me")
	, WindowsNT      (OSFamily.Windows, "windows_nt",    "Windows NT")
	, Windows2000    (OSFamily.Windows, "windows_2000",  "Windows 2000")
	, WindowsXP      (OSFamily.Windows, "windows_xp",    "Windows XP")
	, Windows2003    (OSFamily.Windows, "windows_2003",  "Windows 2003")
	, Windows2008    (OSFamily.Windows, "windows_2008",  "Windows 2008")
	, WindowsVista   (OSFamily.Windows, "windows_vista", "Windows Vista")
	, Windows7       (OSFamily.Windows, "windows_7",     "Windows 7")
	, Windows8       (OSFamily.Windows, "windows_8",     "Windows 8")
	, Windows9       (OSFamily.Windows, "windows_9",     "Windows 9")
	, WindowsCE      (OSFamily.Windows, "windows_ce",    "Windows CE")
	, OS2            (OSFamily.Windows, "os_2",          "OS/2")
	, WindowsUnknown (OSFamily.Windows, OSFamily.Windows.getPlatformPartName())

	
	, MacOSX         (OSFamily.Mac,     "osx",           "Mac OS", "Mac OS X")
	, MacUnknown     (OSFamily.Mac,     OSFamily.Mac.getPlatformPartName())


	, Linux          (OSFamily.Unix,    "linux",         "Linux")
	, MPE_iX         (OSFamily.Unix,    "mpe_ix",        "MPE/iX")
	, HP_UX          (OSFamily.Unix,    "hp_ux",         "HP-UX")
	, AIX            (OSFamily.Unix,    "aix",           "AIX")
	, FreeBSD        (OSFamily.Unix,    "freebsd",       "FreeBSD")
	, Irix           (OSFamily.Unix,    "irix",          "Irix")
	, OS_390         (OSFamily.Unix,    "os390",         "OS/390")
	, DigitalUnix    (OSFamily.Unix,    "digital_unix",  "Digital Unix")
	, Netware_4_11   (OSFamily.Unix,    "netware_4_11",  "NetWare 4.11")
	, OSF1           (OSFamily.Unix,    "osf1",          "OSF1")
	, SunOS          (OSFamily.Unix,    "sunos",         "SunOS")
	, UnixUnknown    (OSFamily.Unix,    OSFamily.Unix.getPlatformPartName())


	, Solaris        (OSFamily.Solaris, "solaris",       "Solaris")
	, SolarisUnknown (OSFamily.Solaris, OSFamily.Solaris.getPlatformPartName())


	, VMS            (OSFamily.VMS,     "openvms",       "OpenVMS")
	, VMSUnknown     (OSFamily.VMS,     OSFamily.VMS.getPlatformPartName())

	;

	//<editor-fold defaultstate="collapsed" desc="Constants">
	public static final String
		NAME = System.getProperty("os.name")
	;
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Variables">
	private static OS systemOS;
	private OSFamily family;
	private String[] variants;
	private String platformPartName;
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Initialization">
	static {
		systemOS = fromName(NAME);
	}

	OS(final OSFamily OSFamily, final String PlatformPartName, final String...Variations) {
		this.family = OSFamily;
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

	public static OSFamily getSystemOSFamily() {
		return systemOS.getFamily();
	}

	public OSFamily getFamily() {
		return family;
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
	public static boolean isPOSIX(final OS OS) {
		return OSFamily.isPOSIX(OS);
	}

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
			return OS.WindowsUnknown;
		else if (lower.contains("mac"))
			return OS.MacUnknown;
		else if (lower.contains("nix") || lower.contains("nux"))
			return OS.UnixUnknown;
		else if (lower.contains("vms"))
			return OS.VMSUnknown;
		else if (lower.contains("solaris"))
			return OS.SolarisUnknown;
		else
			return OS.Unknown;
	}
	//</editor-fold>
}
