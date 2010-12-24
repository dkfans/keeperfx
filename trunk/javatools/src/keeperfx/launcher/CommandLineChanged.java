package keeperfx.launcher;

public interface CommandLineChanged {
	public void updateCommandLine();
	public void setImplicitFlag(String key, boolean val);
}
