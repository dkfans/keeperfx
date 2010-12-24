package keeperfx.launcher.items;

import javax.swing.BorderFactory;
import javax.swing.JPanel;

import keeperfx.configtool.ConfigurationBuffer;
import keeperfx.launcher.CommandLineChanged;

public abstract class CommandLineItem extends JPanel {
	private static final long serialVersionUID = -6982124373973400278L;
	protected final CommandLineChanged commandLineChanged;

	public CommandLineItem(CommandLineChanged commandLineChanged, String label) {
		super();
		this.commandLineChanged = commandLineChanged;
		setBorder(BorderFactory.createTitledBorder(label));
	}
	
	public abstract void load(ConfigurationBuffer config);
	public abstract void save(ConfigurationBuffer config);
}
