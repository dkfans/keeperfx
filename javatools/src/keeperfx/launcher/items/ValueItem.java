package keeperfx.launcher.items;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.BoxLayout;
import javax.swing.JTextField;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;

import keeperfx.configtool.ConfigurationBuffer;
import keeperfx.launcher.CommandLineChanged;

public class ValueItem extends CommandLineItem implements ActionListener, DocumentListener {
	private static final long serialVersionUID = -2382436804714391042L;
	
	private JTextField field;
	private final String key;

	private boolean disableOnDocumentChange;

	public ValueItem(CommandLineChanged commandLineChanged, String label, String key) {
		super(commandLineChanged, label);
		this.key = key;
		this.setLayout(new BoxLayout(this, BoxLayout.X_AXIS));
		
		field = new JTextField();
		field.addActionListener(this);
		field.getDocument().addDocumentListener(this);
		add(field);
	}

	@Override
	public void load(ConfigurationBuffer config) {
		disableOnDocumentChange = true;
		field.setText(config.getItem(key, ""));
		disableOnDocumentChange = false;
	}

	@Override
	public void save(ConfigurationBuffer config) {
		String value = field.getText();
		if (value.equals("")) {
			config.clearItem(key);
		}
		else {
			config.setItem(key, value);
		}
	}

	@Override
	public void actionPerformed(ActionEvent ev) {
		commandLineChanged.updateCommandLine();
	}

	@Override
	public void changedUpdate(DocumentEvent ev) {
	}

	@Override
	public void insertUpdate(DocumentEvent ev) {
		if (!disableOnDocumentChange) {
			commandLineChanged.updateCommandLine();
		}
	}

	@Override
	public void removeUpdate(DocumentEvent ev) {
		if (!disableOnDocumentChange) {
			commandLineChanged.updateCommandLine();
		}
	}
}
