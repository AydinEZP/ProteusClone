# Requirement Coverage Extra PCJ Circuits

این پوشه چند مدار تکمیلی دارد که کنار پکیج اصلی تست‌ها استفاده می‌شوند.

| File | Purpose |
|---|---|
| `09_all_logic_gates_showcase.pcj` | تست AND/OR/XOR/NAND/NOT با ورودی PushButton و خروجی LED |
| `10_battery_passive_interactive_gallery.pcj` | تست Battery، Resistor، LED، Potentiometer، Voltmeter، Capacitor، Inductor و Switch |
| `11_sevensegment_manual_inputs.pcj` | تست سون‌سگمنت با ورودی دستی برای سگمنت‌ها |
| `12_drc_short_vcc_to_gnd.pcj` | تست خطای DRC اتصال مستقیم VCC به GND؛ Run باید متوقف شود |
| `13_drc_floating_logic_input.pcj` | تست Floating input روی گیت AND؛ Run باید اخطار بدهد/متوقف شود |
| `14_drc_contended_outputs.pcj` | تست Contended outputs بین Clock و PushButton روی یک net |
| `15_save_load_full_component_gallery.pcj` | گالری قطعات برای تست Save/Load، rotate/mirror/properties و باز شدن فایل |

