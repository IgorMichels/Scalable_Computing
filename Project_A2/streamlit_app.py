import time
import streamlit as st
import matplotlib.pyplot as plt

from conn_postgres import Connect
conn = Connect('localhost', '', '', '', query=True)

st.set_page_config(
    page_title = 'Traffic Dashboard',
    page_icon = '🚗',
    layout = 'wide',
)

st.title('Live Traffic Dashboard 🚗')
placeholder = st.empty()
while True:
    df_colision = conn.select_colision()
    df_overspeed = conn.select_overspeed()
    df_statistics = conn.select_statistics()
    df_dangerous_driving = conn.select_dangerous_driving()
    df_cars_forbidden = conn.select_cars_forbidden()
    df_historic_info = conn.select_historic_info()
    df_top100 = conn.select_top100()
    df_analysis_time = conn.select_analysis_time()
    with placeholder.container():
        df_col1, df_col2, df_col3, df_col4 = st.columns(4)
        with df_col1: 
            st.markdown('#### 🚗🏎️ Colision risk')
            st.dataframe(df_colision)
        
        with df_col2:
            st.markdown('#### 🏎️ Cars overspeed')
            st.dataframe(df_overspeed)
        
        with df_col3:
            st.markdown('#### 📊 General stats')
            st.dataframe(df_statistics)
        
        with df_col4:
            st.markdown('#### 🚨 Dangerous driving')
            st.dataframe(df_dangerous_driving)

        df_col1, df_col2, df_col3, df_col4 = st.columns(4)
        with df_col1:
            st.markdown('#### 🚫 Cars forbidden')
            st.dataframe(df_cars_forbidden)

        with df_col2:
            st.markdown('#### 📆 Historic data')
            st.dataframe(df_historic_info)
        
        with df_col3:
            st.markdown('#### 🚗 Top 100 cars with most highways')
            st.dataframe(df_top100)
        
        with df_col4:
            st.markdown('#### 🕐 Analysis time')
            st.dataframe(df_analysis_time)
        
        time.sleep(0.1)