package keeperfx.configtool;

import java.awt.BorderLayout;
import java.awt.DisplayMode;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Observable;
import java.util.Observer;

import javax.imageio.ImageIO;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.WindowConstants;

import keeperfx.configtool.items.CensorshipItem;
import keeperfx.configtool.items.ConfigurationItem;
import keeperfx.configtool.items.DisplayModeFilter;
import keeperfx.configtool.items.InstallPathItem;
import keeperfx.configtool.items.MultiChoiceItem;
import keeperfx.configtool.items.ResolutionItem;
import keeperfx.util.ValueObservable;


public class MainWindow extends JFrame
		implements WindowListener, ActionListener, Observer {
	private static final long serialVersionUID = 5350739374145972591L;
	
	private static final File CONFIG_FILE = new File("keeperfx.cfg");
	private String DEFAULT_CONFIG =
		"; Path to the DK files. Usually leaving it as \"./\" will work.\n" +
		"INSTALL_PATH=./\n" +
		"; How many of the game files are on disk; irrelevant option.\n" +
		"INSTALL_TYPE=MAX\n" +
		"; Language definition; sets options for displaying texts.\n" +
		"LANGUAGE=ENG\n" +
		"; Keyboard definition; irrelevant.\n" +
		"KEYBOARD=101\n" +
		"; File format in which screenshots are written; BMP or HSI.\n" +
		"SCREENSHOT=BMP\n" +
		"; Three frontend resolutions: failsafe, movies and menu resolution.\n" +
		"FRONTEND_RES=640x480x8 640x480x8 640x480x8\n" +
		"; List of in-game resolutions. ALT+R will switch between them.\n" +
		"INGAME_RES=640x480x8 800x600x8 1280x1024x8\n" +
		"; Censorship - originally was ON only if language is german.\n" +
		"CENSORSHIP = OFF\n";
	
	//(observable) states
	private final ValueObservable<Boolean> configChanged =
		new ValueObservable<Boolean>(false, ValueObservable.REF_EQUAL_CHECK);
	
	//components
	JPanel centerPanel;
	private JButton saveButton;
	private JButton revertButton;
	private JButton exitButton;

	private final ArrayList<ConfigurationItem> items = new ArrayList<ConfigurationItem>();
	private final ConfigurationBuffer configBuffer = new KeeperFXConfiguration();
	
	private final DisplayModeFilter limitedResolutionFilter = new DisplayModeFilter() {
		@Override
		public boolean allowDisplayMode(DisplayMode mode) {
			if (mode.getBitDepth() != 8) {
				return false;
			}
			
			if (mode.getWidth() >= 300 && mode.getWidth() <= 320 && mode.getHeight() <= 240) {
				return true;
			}
			
			if (mode.getWidth() >= 600 && mode.getWidth() <= 640 && mode.getHeight() <= 480) {
				return true;
			}
			
			return false;
		}
	};
	
	public MainWindow() {
		super("KeeperFX Configuration Tool");
		
		setLocationByPlatform(true);
		setDefaultCloseOperation(WindowConstants.DO_NOTHING_ON_CLOSE);
		addWindowListener(this);
		
		try {
			InputStream iconStream =
				MainWindow.class.getResourceAsStream("/keeperfx.png");
			
			if (iconStream != null) {
				BufferedImage icon = ImageIO.read(iconStream);
				setIconImage(icon);
			}
		} catch (IOException e) {
			//do nothing
		}
		
		initComponents();
		initConfigItems();
		pack();
		loadConfiguration();
		
		configChanged.addObserver(this);
	}

	private void initConfigItems() {
		addConfigItem(new ResolutionItem(configChanged,
				"(Primary) In-Game Resolution:", "INGAME_RES", 0, null));
		addConfigItem(new ResolutionItem(configChanged,
				"Menu Resolution:", "FRONTEND_RES", 2, limitedResolutionFilter));
		addConfigItem(new ResolutionItem(configChanged,
				"Movie Resolution:", "FRONTEND_RES", 1, limitedResolutionFilter));
		addConfigItem(new ResolutionItem(configChanged,
				"Fail-Safe Resolution:", "FRONTEND_RES", 0, limitedResolutionFilter));
		addConfigItem(new MultiChoiceItem(configChanged, "Screenshot Format:",
				"SCREENSHOT", 2, new String[] { "BMP", "HSI" }));
		addConfigItem(new MultiChoiceItem(configChanged, "Language:",
				"LANGUAGE", 4,
				new String[] { "ENG", "ITA", "FRE", "SPA", "DUT", "GER", "POL", "SWE",
				"RUS", "CHI", "CHT", "JAP", }));
		addConfigItem(new InstallPathItem(configChanged));
		addConfigItem(new CensorshipItem(configChanged));
	}

	private void initComponents() {
		setLayout(new BorderLayout());
		
		centerPanel = new JPanel();
		centerPanel.setLayout(new BoxLayout(centerPanel, BoxLayout.Y_AXIS));
		add(centerPanel, BorderLayout.CENTER);
		
		initSouthPanel();
	}
	
	private void addConfigItem(ConfigurationItem item) {
		centerPanel.add(item);
		items.add(item);
	}

	private void initSouthPanel() {
		JPanel southPanel = new JPanel();
		add(southPanel, BorderLayout.SOUTH);
		
		saveButton = new JButton("Save");
		saveButton.addActionListener(this);
		saveButton.setEnabled(false);
		southPanel.add(saveButton);
		
		revertButton = new JButton("Revert");
		revertButton.addActionListener(this);
		revertButton.setEnabled(false);
		southPanel.add(revertButton);
		
		exitButton = new JButton("Exit");
		exitButton.addActionListener(this);
		southPanel.add(exitButton);
	}

	@Override
	public void windowActivated(WindowEvent arg0) {
	}

	@Override
	public void windowClosed(WindowEvent arg0) {
	}

	@Override
	public void windowClosing(WindowEvent ev) {
		tryClose();
	}

	private void tryClose() {
		if (configChanged.getValue()) {
			int result = JOptionPane.showConfirmDialog(this,
					"Unsaved changes in configuration. Do you want to save before exiting?");
			if (result == JOptionPane.CANCEL_OPTION) {
				return;
			}
			else if (result == JOptionPane.YES_OPTION) {
				saveConfiguration();
			}
		}

		dispose();
	}

	private void saveConfiguration() {
		for (ConfigurationItem item : items) {
			item.save(configBuffer);
		}
		
		try {
			configBuffer.saveToStream(new FileOutputStream(CONFIG_FILE));
			configChanged.setValue(false);
		} catch (IOException e) {
			JOptionPane.showMessageDialog(this,
					"Unable to write to " + CONFIG_FILE.getName(), "Error",
					JOptionPane.ERROR_MESSAGE);
		}
	}
	
	private void loadConfiguration() {
		try {
			configBuffer.loadFromStream(new FileInputStream(CONFIG_FILE));
		} catch (FileNotFoundException e) {
			initDefaultConfiguration();
		} catch (IOException e) {
			initDefaultConfiguration();
		}
		
		for (ConfigurationItem item : items) {
			item.load(configBuffer);
		}
		
		configChanged.setValue(false);
	}

	private void initDefaultConfiguration() {
		configBuffer.replace(DEFAULT_CONFIG);
		
		JOptionPane.showMessageDialog(this, CONFIG_FILE.getName() +
				" was not found, initializing to default configuration",
				"Notice", JOptionPane.INFORMATION_MESSAGE);
	}

	@Override
	public void windowDeactivated(WindowEvent arg0) {
	}

	@Override
	public void windowDeiconified(WindowEvent arg0) {
	}

	@Override
	public void windowIconified(WindowEvent arg0) {
	}

	@Override
	public void windowOpened(WindowEvent arg0) {
	}

	@Override
	public void actionPerformed(ActionEvent ev) {
		if (ev.getSource() == saveButton) {
			saveConfiguration();
		}
		else if (ev.getSource() == revertButton) {
			loadConfiguration();
		}
		else if (ev.getSource() == exitButton) {
			tryClose();
		}
	}

	@Override
	public void update(Observable obs, Object arg) {
		if (obs == configChanged) {
			saveButton.setEnabled(configChanged.getValue());
			revertButton.setEnabled(configChanged.getValue());
		}
	}
}
