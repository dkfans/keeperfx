package keeperfx.launcher.items;

import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.IdentityHashMap;

import javax.swing.JCheckBox;

import keeperfx.configtool.ConfigurationBuffer;
import keeperfx.launcher.CommandLineChanged;

public class FlagSetItem extends CommandLineItem implements ActionListener{
	private static final long serialVersionUID = 8051363087994012223L;
	
	private final IdentityHashMap<JCheckBox, String> flagMap;

	public FlagSetItem(CommandLineChanged commandLineChanged, String label,
			int columns, String[] flags, String[] keys) {
		super(commandLineChanged, label);
		assert(flags.length == keys.length);
		
		int rows = (int) Math.ceil(flags.length / (double) columns);
		setLayout(new GridLayout(rows, columns));
		
		flagMap = new IdentityHashMap<JCheckBox, String>();
		for (int i = 0; i < flags.length; ++i) {
			addFlag(flags[i], keys[i]);
		}
	}

	private void addFlag(String flag, String key) {
		JCheckBox checkBox = new JCheckBox(flag);
		checkBox.addActionListener(this);
		add(checkBox);
		flagMap.put(checkBox, key);
	}

	@Override
	public void load(ConfigurationBuffer config) {
		for (JCheckBox checkBox : flagMap.keySet()) {
			String key = flagMap.get(checkBox);
			if (key.charAt(0) == '!') {
				continue;
			}
			
			checkBox.setSelected(config.hasItem(key));
		}
	}

	@Override
	public void save(ConfigurationBuffer config) {
		for (JCheckBox checkBox : flagMap.keySet()) {
			String key = flagMap.get(checkBox);
			if (key.charAt(0) == '!') {
				continue;
			}
			
			if (checkBox.isSelected()) {
				config.setItem(key, null);
			}
			else {
				config.clearItem(key);
			}
		}
	}

	@Override
	public void actionPerformed(ActionEvent ev) {
		JCheckBox checkBox = (JCheckBox) ev.getSource();
		String key = flagMap.get(checkBox);
		
		if (key.charAt(0) == '!') {
			commandLineChanged.setImplicitFlag(key, checkBox.isSelected());
		}
		else {
			commandLineChanged.updateCommandLine();
		}
	}
}
