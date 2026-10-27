import os
import sys
from reportlab.lib.pagesizes import letter
from reportlab.lib import colors
from reportlab.platypus import (
    SimpleDocTemplate, Paragraph, Spacer, Table, TableStyle, PageBreak, KeepTogether, HRFlowable
)
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from pdf_utils import build_pdf

styles = getSampleStyleSheet()

# Custom Palette
COLOR_PRIMARY = colors.HexColor("#0F172A")    # Deep Slate
COLOR_ACCENT = colors.HexColor("#1D4ED8")     # Royal Blue
COLOR_ACCENT_BG = colors.HexColor("#EFF6FF")  # Blue Tint
COLOR_SECONDARY = colors.HexColor("#047857")  # Emerald
COLOR_MUTED = colors.HexColor("#475569")      # Slate text
COLOR_CARD_BG = colors.HexColor("#F8FAFC")    # Light Grey
COLOR_BORDER = colors.HexColor("#CBD5E1")     # Border Slate
COLOR_CODE_BG = colors.HexColor("#1E293B")    # Slate 800
COLOR_CODE_TEXT = colors.HexColor("#38BDF8")  # Sky Blue

# Custom Typography Styles
title_style = ParagraphStyle(
    "DocTitle",
    parent=styles["Normal"],
    fontName="Helvetica-Bold",
    fontSize=20,
    leading=24,
    textColor=COLOR_PRIMARY,
    spaceAfter=6
)

subtitle_style = ParagraphStyle(
    "DocSubTitle",
    parent=styles["Normal"],
    fontName="Helvetica-Bold",
    fontSize=12,
    leading=16,
    textColor=COLOR_ACCENT,
    spaceAfter=14
)

h1_style = ParagraphStyle(
    "H1",
    parent=styles["Normal"],
    fontName="Helvetica-Bold",
    fontSize=13,
    leading=17,
    textColor=COLOR_PRIMARY,
    spaceBefore=12,
    spaceAfter=6,
    keepWithNext=True
)

h2_style = ParagraphStyle(
    "H2",
    parent=styles["Normal"],
    fontName="Helvetica-Bold",
    fontSize=10.5,
    leading=14,
    textColor=COLOR_ACCENT,
    spaceBefore=8,
    spaceAfter=4,
    keepWithNext=True
)

body_style = ParagraphStyle(
    "Body",
    parent=styles["Normal"],
    fontName="Helvetica",
    fontSize=9,
    leading=13,
    textColor=colors.HexColor("#1E293B"),
    spaceAfter=6
)

bullet_style = ParagraphStyle(
    "BulletText",
    parent=body_style,
    leftIndent=14,
    firstLineIndent=-10,
    spaceAfter=4
)

code_style = ParagraphStyle(
    "CodeText",
    parent=styles["Normal"],
    fontName="Courier",
    fontSize=7.8,
    leading=10.5,
    textColor=COLOR_CODE_TEXT
)

callout_style = ParagraphStyle(
    "CalloutText",
    parent=body_style,
    fontSize=8.5,
    leading=12,
    textColor=colors.HexColor("#0F172A")
)

th_style = ParagraphStyle(
    "TH",
    parent=styles["Normal"],
    fontName="Helvetica-Bold",
    fontSize=8,
    leading=10,
    textColor=colors.white
)

td_style = ParagraphStyle(
    "TD",
    parent=styles["Normal"],
    fontName="Helvetica",
    fontSize=7.8,
    leading=10.5,
    textColor=colors.HexColor("#0F172A")
)

td_code_style = ParagraphStyle(
    "TDCode",
    parent=styles["Normal"],
    fontName="Courier-Bold",
    fontSize=7.5,
    leading=10,
    textColor=COLOR_ACCENT
)

def make_callout(text, bg_color=COLOR_ACCENT_BG, border_color=COLOR_ACCENT):
    p = Paragraph(text, callout_style)
    t = Table([[p]], colWidths=[532])
    t.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,-1), bg_color),
        ('BOX', (0,0), (-1,-1), 1, border_color),
        ('TOPPADDING', (0,0), (-1,-1), 6),
        ('BOTTOMPADDING', (0,0), (-1,-1), 6),
        ('LEFTPADDING', (0,0), (-1,-1), 10),
        ('RIGHTPADDING', (0,0), (-1,-1), 10),
    ]))
    return t

def make_code_box(code_text):
    safe_text = code_text.replace('&', '&amp;').replace('<', '&lt;').replace('>', '&gt;').replace('\n', '<br/>').replace(' ', '&nbsp;')
    p = Paragraph(safe_text, code_style)
    t = Table([[p]], colWidths=[532])
    t.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,-1), COLOR_CODE_BG),
        ('BOX', (0,0), (-1,-1), 1, colors.HexColor("#334155")),
        ('TOPPADDING', (0,0), (-1,-1), 6),
        ('BOTTOMPADDING', (0,0), (-1,-1), 6),
        ('LEFTPADDING', (0,0), (-1,-1), 8),
        ('RIGHTPADDING', (0,0), (-1,-1), 8),
    ]))
    return t

def make_meta_card(role_num, title, domain, files_list, presentation_time):
    p_role = Paragraph(f"<b>CANDIDATE / DEFENDER:</b> Person {role_num}", body_style)
    p_title = Paragraph(f"<b>ENGINEERING ROLE:</b> {title}", body_style)
    p_time = Paragraph(f"<b>PRESENTATION TIME:</b> {presentation_time}", body_style)
    p_domain = Paragraph(f"<b>CORE DOMAIN:</b> {domain}", body_style)
    
    files_formatted = "<br/>".join([f"• <b>{f}</b>" for f in files_list])
    p_files = Paragraph(f"<b>ASSIGNED REPOSITORY FILES:</b><br/>{files_formatted}", body_style)
    
    t = Table([
        [p_role, p_time],
        [p_title, ''],
        [p_domain, ''],
        [p_files, '']
    ], colWidths=[360, 172])
    
    t.setStyle(TableStyle([
        ('SPAN', (0, 1), (1, 1)),
        ('SPAN', (0, 2), (1, 2)),
        ('SPAN', (0, 3), (1, 3)),
        ('BACKGROUND', (0,0), (-1,-1), COLOR_CARD_BG),
        ('BOX', (0,0), (-1,-1), 1, COLOR_BORDER),
        ('LINEBELOW', (0,0), (-1,0), 0.5, COLOR_BORDER),
        ('TOPPADDING', (0,0), (-1,-1), 4),
        ('BOTTOMPADDING', (0,0), (-1,-1), 4),
        ('LEFTPADDING', (0,0), (-1,-1), 8),
        ('RIGHTPADDING', (0,0), (-1,-1), 8),
    ]))
    return t

print("Helper builder elements compiled successfully.")
