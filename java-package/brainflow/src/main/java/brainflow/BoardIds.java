package brainflow;

import java.util.HashMap;
import java.util.Map;

/**
 * enum to store all supported boards
 */
public enum BoardIds
{
    PLAYBACK_FILE_BOARD (-3),
    STREAMING_BOARD (-2),
    SYNTHETIC_BOARD (-1),
    CYTON_BOARD (0),
    GANGLION_BOARD (1),
    CYTON_DAISY_BOARD (2),
    GALEA_BOARD (3),
    GANGLION_WIFI_BOARD (4),
    CYTON_WIFI_BOARD (5),
    CYTON_DAISY_WIFI_BOARD (6),
    BRAINBIT_BOARD (7),
    UNICORN_BOARD (8),
    CALLIBRI_EEG_BOARD (9),
    CALLIBRI_EMG_BOARD (10),
    CALLIBRI_ECG_BOARD (11),
    FASCIA_BOARD (12),
    NOTION_1_BOARD (13),
    NOTION_2_BOARD (14),
    IRONBCI_BOARD (15),
    GFORCE_PRO_BOARD (16),
    FREEEEG32_BOARD (17),
    BRAINBIT_BLED_BOARD (18),
    GFORCE_DUAL_BOARD (19),
    GALEA_SERIAL_BOARD (20),
    MUSE_S_BLED_BOARD (21),
    MUSE_2_BLED_BOARD (22),
    CROWN_BOARD (23),
    ANT_NEURO_EE_411_BOARD (24);

    private final int board_id;
    private static final Map<Integer, BoardIds> bi_map = new HashMap<Integer, BoardIds> ();

    public int get_code ()
    {
        return board_id;
    }

    public static String string_from_code (final int code)
    {
        return from_code (code).name ();
    }

    public static BoardIds from_code (final int code)
    {
        final BoardIds element = bi_map.get (code);
        return element;
    }

    BoardIds (final int code)
    {
        board_id = code;
    }

    static
    {
        for (final BoardIds bi : BoardIds.values ())
        {
            bi_map.put (bi.get_code (), bi);
        }
    }
}
