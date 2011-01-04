package keeperfx.launcher;

import java.awt.BorderLayout;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;
import java.awt.image.BufferedImage;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.PrintStream;
import java.util.ArrayList;

import javax.imageio.ImageIO;
import javax.swing.BorderFactory;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JTextField;
import javax.swing.WindowConstants;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;

import keeperfx.configtool.ConfigurationBuffer;
import keeperfx.configtool.KeeperFXConfigTool;
import keeperfx.launcher.items.CommandLineItem;
import keeperfx.launcher.items.FlagSetItem;
import keeperfx.launcher.items.ValueItem;

public class MainWindow extends JFrame implements ActionListener, DocumentListener,
		CommandLineChanged, WindowListener {
	private static final long serialVersionUID = 1527215476970706506L;
	private static final File COMMAND_LINE_SAVE_FILE = new File("SavedCommandLine.txt");
	
	//components
	JPanel centerPanel;
	private JTextField commandLineField;
	private JButton launchButton;
	private JButton editConfigButton;
	private JButton exitButton;

	private final ArrayList<CommandLineItem> items = new ArrayList<CommandLineItem>();
	private final ConfigurationBuffer configBuffer = new KeeperFXCommandLine();
	private boolean disableOnDocumentChange;

	private boolean debugExecutable;

	public MainWindow() {
		super("KeeperFX Launcher Tool");
		
		setLocationByPlatform(true);
		setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
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
		initCommandLineItems();
		pack();
		
		try {
			BufferedReader reader = new BufferedReader(new FileReader(COMMAND_LINE_SAVE_FILE));
			commandLineField.setText(reader.readLine());
			reader.close();
		} catch (FileNotFoundException e) {
			//do nothing
		} catch (IOException e) {
			//do nothing
		}
	}

	private void initCommandLineItems() {
		final String[] flags = {
				"Debug Executable",
				"1 Player",
				"No Intro Movie",
				"No CD Check",
				"No Sound",
				"Alt. Input Mode",
				"Disable AWE32/64",
				"Easter Egg",
				"Quick Map 1",
				"Smooth Video",
				"Column Convert",
				"Light Convert",
		};
		final String[] flagKeys = {
				"!debug", //! means this is an implicit flag not seen on command line
				"1player",
				"nointro",
				"nocd",
				"nosound",
				"altinput",
				"usersfont",
				"alex",
				"q",
				"vidsmooth",
				"columnconvert",
				"lightconvert",
		};
		
		addCommandLineItem(new FlagSetItem(this, "Flags:", 2, flags, flagKeys), centerPanel);
		addCommandLineItem(new ValueItem(this, "Frames/Second (Game Speed):", "fps"), centerPanel);
		
		JPanel fieldPanel = new JPanel(new GridLayout(2, 2));
		centerPanel.add(fieldPanel);
		addCommandLineItem(new ValueItem(this, "Start Level:", "level"), fieldPanel);
		addCommandLineItem(new ValueItem(this, "Human Player:", "human"), fieldPanel);
		addCommandLineItem(new ValueItem(this, "Packet File to Save:", "packetsave"), fieldPanel);
		addCommandLineItem(new ValueItem(this, "Packet File to Load:", "packetload"), fieldPanel);
		
		addCommandLineItem(new ValueItem(this, "TCP/IP Sessions (e.g. 127.0.0.1:5555):", "sessions"), centerPanel);
	}

	private void initComponents() {
		setLayout(new BorderLayout());
		
		JPanel northPanel = new JPanel();
		northPanel.setLayout(new BoxLayout(northPanel, BoxLayout.X_AXIS));
		northPanel.setBorder(BorderFactory.createTitledBorder("Command Line:"));
		add(northPanel, BorderLayout.NORTH);
		
		commandLineField = new JTextField();
		commandLineField.addActionListener(this);
		commandLineField.getDocument().addDocumentListener(this);
		northPanel.add(commandLineField);
		
		centerPanel = new JPanel();
		centerPanel.setLayout(new BoxLayout(centerPanel, BoxLayout.Y_AXIS));
		add(centerPanel, BorderLayout.CENTER);
		
		initSouthPanel();
	}
	
	private void addCommandLineItem(CommandLineItem item, JPanel panel) {
		panel.add(item);
		items.add(item);
	}

	private void initSouthPanel() {
		JPanel southPanel = new JPanel();
		add(southPanel, BorderLayout.SOUTH);
		
		launchButton = new JButton("Launch");
		launchButton.addActionListener(this);
		southPanel.add(launchButton);
		
		editConfigButton = new JButton("Edit Configuration...");
		editConfigButton.addActionListener(this);
		southPanel.add(editConfigButton);
		
		exitButton = new JButton("Exit");
		exitButton.addActionListener(this);
		southPanel.add(exitButton);
	}

	@Override
	public void actionPerformed(ActionEvent ev) {
		if (ev.getSource() == launchButton) {
			launch();
		}
		else if (ev.getSource() == editConfigButton) {
			KeeperFXConfigTool.main(null);
		}
		else if (ev.getSource() == exitButton) {
			dispose();
		}
		else if (ev.getSource() == commandLineField) {
			parseCommandLine();
		}
	}
	
	private void launch() {
		String command;
		if (debugExecutable) {
			command = "keeperfx_dbg.exe";
		}
		else {
			command = "keeperfx.exe";
		}
		
		command += ' ';
		command += commandLineField.getText();
		command = command.trim();
		
		try {
			Runtime.getRuntime().exec(command);
		} catch (IOException e) {
			JOptionPane.showMessageDialog(this, "Could not execute command '" + command + "'" +
					"\nProbably executable could not be found (is this application running from KeeperFX directory?)",
					"Error", JOptionPane.ERROR_MESSAGE);
		}
	}

	private void parseCommandLine() {
		configBuffer.replace(commandLineField.getText());
		
		for (CommandLineItem item : items) {
			item.load(configBuffer);
		}
	}
	
	@Override
	public void updateCommandLine() {
		for (CommandLineItem item : items) {
			item.save(configBuffer);
		}
		
		disableOnDocumentChange = true;
		commandLineField.setText(configBuffer.toString().trim());
		disableOnDocumentChange = false;
	}

	@Override
	public void changedUpdate(DocumentEvent ev) {
	}

	@Override
	public void insertUpdate(DocumentEvent ev) {
		if (!disableOnDocumentChange) {
			parseCommandLine();
		}
	}

	@Override
	public void removeUpdate(DocumentEvent ev) {
		if (!disableOnDocumentChange) {
			parseCommandLine();
		}
	}

	@Override
	public void setImplicitFlag(String key, boolean val) {
		if (key.equals("!debug")) {
			debugExecutable = val;
			launchButton.setText(val? "Debug" : "Launch");
		}
	}

	@Override
	public void windowActivated(WindowEvent ev) {}

	@Override
	public void windowClosed(WindowEvent ev) {
		try {
			PrintStream stream = new PrintStream(new FileOutputStream(COMMAND_LINE_SAVE_FILE));
			stream.print(commandLineField.getText());
			stream.close();
		} catch (FileNotFoundException e) {
			//do nothing
		}
	}

	@Override
	public void windowClosing(WindowEvent ev) {}

	@Override
	public void windowDeactivated(WindowEvent ev) {}

	@Override
	public void windowDeiconified(WindowEvent ev) {}

	@Override
	public void windowIconified(WindowEvent ev) {}

	@Override
	public void windowOpened(WindowEvent ev) {}
}
